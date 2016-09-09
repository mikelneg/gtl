#pragma warning(disable : 4503)

#include "gtl/physics_simulation.h"

#include <gtl/swap_vector.h>
#include <vn/swap_object.h>

#include <Box2D/Box2D.h>
#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstdint>

#include <Eigen/Core>
#include <Eigen/Geometry>

//#include <boost/units/quantity.hpp>
//#include <boost/units/systems/si/area.hpp>

#include <boost/variant.hpp>
#include <vn/math_utilities.h>

/*-----------------------------------------------------------------------------
    Mikel Negugogor (http://github.com/mikelneg)    
-----------------------------------------------------------------------------*/

namespace gtl {

namespace {

    struct collision_category {
        enum : uint16_t {
            BOUNDARY = 1,
            ENTITY = 2,
            SENSOR = 4
        };
    };

    template <typename T, typename U>
    static void simulation_thread(vn::swap_object<T>&,
        gtl::swap_vector<U>& physics_task_queue_,
        std::atomic_flag& quit_);

    template <typename T>
    struct query_callback_helper : b2QueryCallback, T {
        query_callback_helper(T t)
            : T(t)
        {
        }
        bool ReportFixture(b2Fixture* fixture_) override
        {
            return (T::operator())(fixture_);
        }
    };

    template <typename T>
    auto make_query_callback(T t) { return query_callback_helper<T>{ std::move(t) }; }

    //inline
    //void* to_user_data(b2Body* p) noexcept {
    //    return p;
    //}
    //
    //inline
    //b2Body* from_user_data(void* p) noexcept {
    //    return reinterpret_cast<b2Body*>(p);
    //}

    using map_type = std::multimap<uint16_t, b2Body*>;

    struct box2d_generator_visitor : boost::static_visitor<b2Body*> {

        b2World& world_;

        map_type& entity_map_;

        box2d_generator_visitor(b2World& w_, map_type& map_) noexcept : world_{ w_ }, entity_map_{ map_ } {}

        template <typename T>
        b2Body* operator()(T const& t) const
        {
            assert(false);
            return nullptr;
        }

        b2Body* operator()(gtl::physics::generators::static_box const& o) const
        {

            using namespace boost::units;

            b2BodyDef body_;

            body_.position = b2Vec2{ o.xy_.first / si::meters, o.xy_.second / si::meters };
            body_.angle = o.angle_ / si::radians;
            body_.userData = reinterpret_cast<void*>(o.info_.value());
            body_.type = b2_staticBody;
            //body_.awake = true; // default
            // etc..

            b2Body* ptr = world_.CreateBody(&body_);

            b2PolygonShape shape_;
            shape_.SetAsBox(o.wh_.first / si::meters, o.wh_.second / si::meters);
            //shape_.SetAsBox(o.wh_.first.value(),o.wh_.second.value());//,b2Vec2{o.x,o.y},o.a);

            // TODO b2EdgeShape instead of box..

            b2FixtureDef fixture_;

            fixture_.filter.categoryBits = collision_category::BOUNDARY;
            fixture_.filter.maskBits = collision_category::ENTITY;
            fixture_.shape = &shape_;

            fixture_.userData = reinterpret_cast<void*>(ptr);

            ptr->CreateFixture(&fixture_);

            return ptr;
        }

        b2Body* operator()(gtl::physics::generators::dynamic_jointed_boxes const& o) const
        {

            using namespace boost::units;
            if (o.boxes_.size() < 2) {
                assert(false);
                return nullptr;
            }

            auto create_body = [&](auto& a, auto const& userdata) {
                b2BodyDef d;
                d.position.Set(a.xy_.first / si::meters, a.xy_.second / si::meters);
                d.angle = a.angle_ / si::radians;
                d.userData = userdata;
                d.type = b2_dynamicBody;
                return world_.CreateBody(&d);
            };

            auto do_the_rest = [&](auto& a, auto& body, auto const& userdata) {
                b2PolygonShape s;
                s.SetAsBox(0.5f * (a.wh_.first / si::meters), 0.5f * (a.wh_.second / si::meters));
                //s.SetAsBox(a.wh_.first / si::meters,a.wh_.second / si::meters);

                b2FixtureDef f;
                f.filter.categoryBits = collision_category::ENTITY;
                f.filter.maskBits = collision_category::ENTITY | collision_category::SENSOR | collision_category::BOUNDARY;
                f.shape = &s;
                f.userData = userdata;

                f.isSensor = false;
                f.restitution = 0.46f;
                f.density = 1.0f; // default numbers from the docs, revisit later..
                f.friction = 0.3f;

                body->CreateFixture(&f);

                body->SetLinearDamping(0.4f); // more completely invented numbers..
                body->SetAngularDamping(0.5f);
                body->SetSleepingAllowed(true);

                body->ApplyLinearImpulse(b2Vec2{ std::cos(a.angle_ / si::radians),
                                             std::sin(a.angle_ / si::radians) },
                    body->GetWorldCenter(), true);
            };

            auto connect_bodies = [&](auto& a, auto& b) {
                b2RevoluteJointDef d;
                d.bodyA = a;
                d.bodyB = b;
                d.collideConnected = false;

                d.localAnchorA = b2Vec2{ 0.0f, -0.5f };
                d.localAnchorB = b2Vec2{ 0.0f, 0.5f };

                d.enableLimit = true;

                float const pi_180 = (std::atan(1.0f) * 4.0f) / 180.0f;

                d.referenceAngle = 0.0f * pi_180;
                d.lowerAngle = -45.0f * pi_180;
                d.upperAngle = 45.0f * pi_180;

                world_.CreateJoint(&d);
            };

            std::vector<b2Body*> body_ptrs_;
            for (auto&& b : o.boxes_) {
                body_ptrs_.emplace_back(create_body(b, reinterpret_cast<void*>(b.info_.value())));
            }

            //entity_map_.emplace(o.boxes_[0].id, body_ptrs_[0]);

            for (unsigned i = 0; i < body_ptrs_.size(); ++i) {
                do_the_rest(o.boxes_[i], body_ptrs_[i], body_ptrs_[0]); // all fixtures store the b2Body* for the root
                entity_map_.emplace(o.boxes_[0].info_.entity_id(), body_ptrs_[i]);
            }

            auto n = std::begin(body_ptrs_);
            auto m = n;
            auto e = std::end(body_ptrs_);
            if (m != e)
                std::advance(m, 1);

            for (; m != e; ++n, ++m) {
                connect_bodies(*n, *m);
            }

            return body_ptrs_[0];
        }

        b2Body* operator()(gtl::physics::generators::dynamic_box const& o) const
        {

            using namespace boost::units;

            b2BodyDef body_;

            body_.position = b2Vec2{ o.xy_.first / si::meters, o.xy_.second / si::meters };
            body_.angle = o.angle_ / si::radians;
            body_.userData = reinterpret_cast<void*>(o.info_.value());
            body_.type = b2_dynamicBody;
            //body_.awake = true; // default
            // etc..

            b2Body* ptr = world_.CreateBody(&body_);

            entity_map_.emplace(o.info_.entity_id(), ptr);

            b2PolygonShape shape_;
            shape_.SetAsBox(o.wh_.first / si::meters, o.wh_.second / si::meters);
            //shape_.SetAsBox(o.wh_.first.value(),o.wh_.second.value());//,b2Vec2{o.x,o.y},o.a);

            b2FixtureDef fixture_;

            fixture_.filter.categoryBits = collision_category::ENTITY;
            fixture_.filter.maskBits = collision_category::ENTITY | collision_category::SENSOR | collision_category::BOUNDARY;
            fixture_.shape = &shape_;
            fixture_.userData = reinterpret_cast<void*>(ptr);

            fixture_.isSensor = false;
            fixture_.restitution = 0.46f;
            fixture_.density = 1.0f; // default numbers from the docs, revisit later..
            fixture_.friction = 0.3f;

            ptr->CreateFixture(&fixture_);

            ptr->SetLinearDamping(0.4f); // more completely invented numbers..
            ptr->SetAngularDamping(0.5f);
            ptr->SetSleepingAllowed(true);

            ptr->ApplyLinearImpulse(b2Vec2{ std::cos(o.angle_ / si::radians),
                                        std::sin(o.angle_ / si::radians) },
                ptr->GetLocalCenter(), true);

            return ptr;
        }

        b2Body* operator()(gtl::physics::generators::destroy_object_implode const& o) const
        {

            auto range = entity_map_.equal_range(o.id);
            if (range.first == range.second) {
                return nullptr;
            }

            b2AABB body_aabb_{ b2Vec2{ FLT_MAX, FLT_MAX }, b2Vec2{ -FLT_MAX, -FLT_MAX } };

            for (auto it = range.first; it != range.second; ++it) {
                b2Body* body_ptr_ = it->second;
                b2Fixture* fixture_ = body_ptr_->GetFixtureList();
                while (fixture_) {
                    body_aabb_.Combine(body_aabb_, fixture_->GetAABB(0)); // check this index for "chained" fixtures..
                    fixture_ = fixture_->GetNext();
                }
                body_aabb_.lowerBound -= b2Vec2{ 12.0f, 12.0f };
                body_aabb_.upperBound += b2Vec2{ 12.0f, 12.0f };
            }

            auto implode_around_AABB = make_query_callback(
                [&](b2Fixture* fixture_) {
                    b2Body* b = fixture_->GetBody();
                    b->SetAwake(true);
                    b2Vec2 vec_{ body_aabb_.GetCenter() - b->GetWorldCenter() };
                    float mag_ = vec_.Normalize();
                    vec_ *= ((1000.0f * b->GetMass()) / mag_);
                    b->ApplyForceToCenter(vec_, true);
                    return true; // continue the query
                });

            for (auto it = range.first; it != range.second; ++it) {
                world_.DestroyBody(it->second);
            }

            entity_map_.erase(o.id);

            world_.QueryAABB(&implode_around_AABB, body_aabb_);

            return nullptr;
        }

        b2Body* operator()(gtl::physics::generators::boost_object const& o) const
        {

            auto range = entity_map_.equal_range(o.id);
            if (range.first == range.second) {
                return nullptr;
            }

            b2Body* main_body_ = (range.first)->second;
            main_body_->SetAwake(true);

            main_body_->ApplyLinearImpulse(b2Vec2{ 100.0f * std::cos(-1.0f * main_body_->GetAngle()),
                                               100.0f * std::sin(-1.0f * main_body_->GetAngle()) },
                main_body_->GetWorldCenter(), true);
            //main_body_->GetPosition(), true);
            return nullptr;
        }

        b2Body* operator()(gtl::physics::generators::boost_object_vec const& o) const
        {

            auto range = entity_map_.equal_range(o.id);
            if (range.first == range.second) {
                return nullptr;
            }

            b2Body* main_body_ = (range.first)->second;
            main_body_->SetAwake(true);

            main_body_->ApplyLinearImpulse(b2Vec2{ o.x, o.y }, // o.x / 100.0f,o.y / 100.0f},
                main_body_->GetWorldCenter(), true);
            //main_body_->GetPosition(), true);
            return nullptr;
        }

        b2Body* operator()(gtl::physics::generators::drive_object_vec const& o) const
        {

            auto range = entity_map_.equal_range(o.id);
            if (range.first == range.second) {
                return nullptr;
            }

            b2Body* main_body_ = (range.first)->second;
            main_body_->SetAwake(true);

            main_body_->ApplyForce(b2Vec2{ o.x, o.y }, main_body_->GetWorldCenter(), true);

            //main_body_->ApplyLinearImpulse(b2Vec2{o.x,o.y},// o.x / 100.0f,o.y / 100.0f},
            //                                      main_body_->GetWorldCenter(), true);
            //main_body_->GetPosition(), true);
            return nullptr;
        }
    };
}

physics_simulation::physics_simulation(gtl::swap_vector<gtl::physics::generator>& tasks_) //boost::units::quantity<boost::units::si::area> area_,
//std::vector<Eigen::Vector4f> positions_)

{
    quit_.test_and_set();
    //thread_ = std::thread{&simulation_thread<render_data,gtl::physics::generator>,std::ref(entities_),std::ref(tasks_),std::ref(quit_)};
    thread_ = std::thread{ &simulation_thread<render_data, gtl::physics::generator>,
        std::ref(render_data_), std::ref(tasks_), std::ref(quit_) };
}

namespace {

    template <typename T, typename U>
    static void simulation_thread(vn::swap_object<T>& rend_data_,
        gtl::swap_vector<U>& physics_task_queue_,
        //std::vector<physics::generator> generators_,
        std::atomic_flag& quit_)
    {
        //b2World world_{b2Vec2{0.0f,-9.8f}};
        b2World world_{ b2Vec2{ 0.0f, 0.0f } };
        map_type map_;

        world_.SetAllowSleeping(true);

        box2d_generator_visitor visitor_{ world_, map_ };

        auto task_local_ = physics_task_queue_.make_vector();
        physics_task_queue_.swap_out(task_local_);

        for (auto&& e : task_local_) {
            apply_visitor(visitor_, e);
        }

        float32 timeStep = 1.0f / 60.0f;
        int32 velocityIterations = 8;
        int32 positionIterations = 3;

        render_data local_rend_data_;

        auto& positions_ = local_rend_data_.entities_;
        auto& bones_ = local_rend_data_.bones_;

        //std::vector<T> positions_;
        //positions_.reserve(100);

        std::vector<b2Body*> selected_bodies_;
        selected_bodies_.reserve(100);

        int64_t max_wait_ = 0;

        auto query_bodies = [&]() {
            //positions_.clear();
            for (b2Body* b = world_.GetBodyList(); b; b = b->GetNext()) {
                b2Vec2 position = b->GetPosition();
                //uint32_t index_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b->GetUserData()));
                // temporar

                //positions_.emplace_back(InstanceInfo{{position.x,
                //                           position.y,
                //                           1.0f,1.0f},
                //                           0, // MESH_ID
                //                           index_});
            }
            //swapvec_.swap_in(positions_);
        };

        auto query_around_AABB = make_query_callback(
            [&](b2Fixture* fixture_) {
                //b2Body *b = fixture_->GetBody();
                //b->SetAwake(true);
                //b2Vec2 position = b->GetPosition();
                //uint32_t index_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b->GetUserData()));
                //
                ////b2AABB aabb_ = fixture_->GetAABB(0);
                //
                //positions_.emplace_back(InstanceInfo{{position.x,
                //                           position.y,
                //                           1.0f,1.0f},
                //                           {1.0f, 1.0f, 1.0f, b->GetAngle()},index_});
                ////positions_.emplace_back(T{{position.x,
                ////                           position.y,
                ////                           aabb_.GetExtents().x,aabb_.GetExtents().y},
                ////                           {1.0f, 1.0f, 1.0f, 0.0f},index_});
                //return true; // continue the query

                b2Body& body_ = *(fixture_->GetBody());
                body_.SetAwake(true);

                //uint32_t index_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body_.GetUserData()));
                b2Vec2 const& p = body_.GetPosition();

                //positions_.emplace_back(InstanceInfo{{p.x, p.y, 1.0f, 1.0f},
                //                                   //{1.0f, 1.0f, 1.0f, body_.GetAngle()},
                //                                   0, // MESH_ID
                //                                   static_cast<uint32_t>(bones_.size()),
                //                                   index_});
                // single body addition..
                Eigen::Matrix4f m = (Eigen::Affine3f{ Eigen::Translation3f{ Eigen::Vector3f{ p.x, p.y, 0.1f } } } // HACK specify plane (0.1f) elsewhere..
                    * Eigen::Affine3f{ Eigen::AngleAxisf{ body_.GetAngle(), Eigen::Vector3f{ 0.0f, 0.0f, 1.0f } } })
                                        .matrix(); //Eigen::Rotation2Df{body_.GetAngle()}.matrix();

                bones_.emplace_back(m.transpose());

                return true;
            });

        auto dump_fixtures = [&](b2Body& body_) {
            InstanceInfo instance{ reinterpret_cast<uintptr_t>(body_.GetUserData()),
                static_cast<uint16_t>(bones_.size()) };

            b2Vec2 const& p = body_.GetPosition();

            positions_.emplace_back(instance);

            // process all bodies that participate in the entity..
            auto range = map_.equal_range(instance.entity_id());
            for (auto it = range.first; it != range.second; ++it) {
                b2Vec2 const& p = it->second->GetPosition();
                float const& angle = it->second->GetAngle();

                Eigen::Matrix4f m = (Eigen::Affine3f{ Eigen::Translation3f{ Eigen::Vector3f{ p.x, p.y, 0.1f } } }
                    * Eigen::Affine3f{ Eigen::AngleAxisf{ angle,
                          Eigen::Vector3f{ 0.0f, 0.0f, 1.0f } } })
                                        .matrix();

                bones_.emplace_back(m.transpose());
            }
        };

        auto query_bodies_around_AABB = make_query_callback(
            [&](b2Fixture* fixture_) {
                b2Body* const body_ptr_ = reinterpret_cast<b2Body*>(fixture_->GetUserData());
                body_ptr_->SetAwake(true);
                selected_bodies_.emplace_back(body_ptr_);
                return true; // continue the query
            });

        b2AABB aabb;
        aabb.lowerBound.Set(-60.0f, -60.0f);
        aabb.upperBound.Set(60.0f, 60.0f);

        auto time_ = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(15);

        while (quit_.test_and_set(std::memory_order_acquire)) {

            bones_.clear();
            positions_.clear();
            selected_bodies_.clear();

            world_.QueryAABB(&query_bodies_around_AABB, aabb);

            std::sort(begin(selected_bodies_), end(selected_bodies_));
            auto new_end_ = std::unique(begin(selected_bodies_), end(selected_bodies_));
            selected_bodies_.erase(new_end_, end(selected_bodies_));

            for (auto&& e : selected_bodies_) {
                dump_fixtures(*e);
            }

            // all bodies as objects..
            //world_.QueryAABB(&query_around_AABB, aabb);

            //query_bodies();
            //swapvec_.swap_in(positions_);

            rend_data_.swap_in(local_rend_data_);

            task_local_.clear();
            physics_task_queue_.swap_out(task_local_);
            for (auto&& e : task_local_) {
                apply_visitor(visitor_, e);
            }

            world_.Step(timeStep, velocityIterations, positionIterations);

            auto now_ = std::chrono::high_resolution_clock::now();
            if ((time_ - now_) > std::chrono::milliseconds(1)) {
                std::this_thread::sleep_for(time_ - now_);
            }
            time_ = now_ + std::chrono::milliseconds(5); // TODO arbitrary values, revist..
        }

        std::cout << "physics thread exiting, max wait == " << max_wait_ << "\n";
    }

} // namespace
} // namespace