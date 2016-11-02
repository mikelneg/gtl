/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#pragma warning(disable : 4503)

#include "gtl/physics/simulation_dummy_impl.h"

#include <gtl/physics/command_variant.h>

//#include <gtl/swap_vector.h>
#include <gtl/rate_limiter.h>

#include <vn/single_consumer_queue.h>

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
#include <gtl/draw_kit.h>

// Bullet physics

#pragma warning(disable : 4305)
#include <btBulletDynamicsCommon.h>

// Link order:
// BulletMultiThreaded (optional)
// MiniCL (optional)
// BulletWorldImporter (optional)
// BulletSoftBody (optional)
// BulletDynamics
// BulletCollision
// LinearMath

namespace gtl {

namespace {

    struct collision_category {
        enum : uint16_t { BOUNDARY = 1, ENTITY = 2, SENSOR = 4 };
    };

    template <typename T, typename U>
    static void bullet_simulation_thread(vn::swap_object<T>& rend_data_, vn::single_consumer_queue<U>& physics_task_queue_, std::atomic_flag& quit_,
                                         gtl::draw_kit& b2d_adapter_);

    template <typename T, typename U>
    static void simulation_thread(vn::swap_object<T>&, vn::single_consumer_queue<U>& physics_task_queue_, std::atomic_flag& quit_, gtl::draw_kit&);

    template <typename T>
    struct query_callback_helper : b2QueryCallback, T {
        query_callback_helper(T t) : T(t)
        {
        }
        bool ReportFixture(b2Fixture* fixture_) override
        {
            return (T::operator())(fixture_);
        }
    };

    template <typename T>
    auto make_query_callback(T t)
    {
        return query_callback_helper<T>{std::move(t)};
    }

    // inline
    // void* to_user_data(b2Body* p) noexcept {
    //    return p;
    //}
    //
    // inline
    // b2Body* from_user_data(void* p) noexcept {
    //    return reinterpret_cast<b2Body*>(p);
    //}

    using map_type = std::multimap<uint16_t, b2Body*>;

    struct box2d_generator_visitor : boost::static_visitor<b2Body*> {

        b2World& world_;

        map_type& entity_map_;

        box2d_generator_visitor(b2World& w_, map_type& map_) noexcept : world_{w_}, entity_map_{map_}
        {
        }

        template <typename T>
        b2Body* operator()(T const& t) const
        {
            assert(false);
            return nullptr;
        }

        b2Body* operator()(gtl::physics::commands::polymorphic_generator const& o) const
        {
            o.generator_->apply(world_);
            return nullptr;
        }

        b2Body* operator()(gtl::physics::commands::static_rectangle const& o) const
        {

            using namespace boost::units;

            b2BodyDef body_;

            body_.position = b2Vec2{o.xy_.first / si::meters, o.xy_.second / si::meters};
            body_.angle = o.angle_ / si::radians;
            body_.userData = reinterpret_cast<void*>(o.info_.value());
            body_.type = b2_staticBody;
            // body_.awake = true; // default
            // etc..

            b2Body* ptr = world_.CreateBody(&body_);

            b2PolygonShape shape_;
            shape_.SetAsBox(o.wh_.first / si::meters, o.wh_.second / si::meters);
            // shape_.SetAsBox(o.wh_.first.value(),o.wh_.second.value());//,b2Vec2{o.x,o.y},o.a);

            // TODO b2EdgeShape instead of box..

            b2FixtureDef fixture_;

            fixture_.isSensor = false;
            fixture_.restitution = 0.46f;
            fixture_.density = 1.0f; // default numbers from the docs, revisit later..
            fixture_.friction = 0.1f;

            fixture_.filter.categoryBits = collision_category::BOUNDARY;
            fixture_.filter.maskBits = collision_category::ENTITY;
            fixture_.shape = &shape_;

            fixture_.userData = reinterpret_cast<void*>(ptr);

            ptr->CreateFixture(&fixture_);

            return ptr;
        }

        b2Body* operator()(gtl::physics::commands::dynamic_jointed_rectangles const& o) const
        {

            using namespace boost::units;
            if (o.boxes_.size() < 2)
            {
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
                // s.SetAsBox(a.wh_.first / si::meters,a.wh_.second / si::meters);

                b2FixtureDef f;
                f.filter.categoryBits = collision_category::ENTITY;
                f.filter.maskBits = collision_category::ENTITY | collision_category::SENSOR | collision_category::BOUNDARY;
                f.shape = &s;
                f.userData = userdata;

                f.isSensor = false;
                f.restitution = 0.46f;
                f.density = 1.0f; // default numbers from the docs, revisit later..
                f.friction = 0.0f;

                body->CreateFixture(&f);

                body->SetLinearDamping(0.4f); // more completely invented numbers..
                body->SetAngularDamping(0.5f);
                body->SetSleepingAllowed(true);

                body->ApplyLinearImpulse(b2Vec2{std::cos(a.angle_ / si::radians), std::sin(a.angle_ / si::radians)}, body->GetWorldCenter(), true);
            };

            auto connect_bodies = [&](auto& a, auto& b) {
                b2RevoluteJointDef d;
                d.bodyA = a;
                d.bodyB = b;
                d.collideConnected = false;

                d.localAnchorA = b2Vec2{0.0f, -0.5f};
                d.localAnchorB = b2Vec2{0.0f, 0.5f};

                d.enableLimit = true;

                float const pi_180 = (std::atan(1.0f) * 4.0f) / 180.0f;

                d.referenceAngle = 0.0f * pi_180;
                d.lowerAngle = -45.0f * pi_180;
                d.upperAngle = 45.0f * pi_180;

                world_.CreateJoint(&d);
            };

            std::vector<b2Body*> body_ptrs_;
            for (auto&& b : o.boxes_)
            {
                body_ptrs_.emplace_back(create_body(b, reinterpret_cast<void*>(b.info_.value())));
            }

            // entity_map_.emplace(o.boxes_[0].id, body_ptrs_[0]);

            for (unsigned i = 0; i < body_ptrs_.size(); ++i)
            {
                do_the_rest(o.boxes_[i], body_ptrs_[i], body_ptrs_[0]); // all fixtures store the b2Body* for the root
                entity_map_.emplace(o.boxes_[0].info_.entity_id(), body_ptrs_[i]);
            }

            auto n = std::begin(body_ptrs_);
            auto m = n;
            auto e = std::end(body_ptrs_);
            if (m != e)
                std::advance(m, 1);

            for (; m != e; ++n, ++m)
            {
                connect_bodies(*n, *m);
            }

            return body_ptrs_[0];
        }

        b2Body* operator()(gtl::physics::commands::dynamic_rectangle const& o) const
        {

            using namespace boost::units;

            b2BodyDef body_;

            body_.position = b2Vec2{o.xy_.first / si::meters, o.xy_.second / si::meters};
            body_.angle = o.angle_ / si::radians;
            body_.userData = reinterpret_cast<void*>(o.info_.value());
            body_.type = b2_dynamicBody;
            // body_.awake = true; // default
            // etc..

            b2Body* ptr = world_.CreateBody(&body_);

            entity_map_.emplace(o.info_.entity_id(), ptr);

            b2PolygonShape shape_;
            shape_.SetAsBox(o.wh_.first / si::meters, o.wh_.second / si::meters);
            // shape_.SetAsBox(o.wh_.first.value(),o.wh_.second.value());//,b2Vec2{o.x,o.y},o.a);

            b2FixtureDef fixture_;

            fixture_.filter.categoryBits = collision_category::ENTITY;
            fixture_.filter.maskBits = collision_category::ENTITY | collision_category::SENSOR | collision_category::BOUNDARY;
            fixture_.shape = &shape_;
            fixture_.userData = reinterpret_cast<void*>(ptr);

            fixture_.isSensor = false;
            fixture_.restitution = 0.46f;
            fixture_.density = 1.0f; // default numbers from the docs, revisit later..
            fixture_.friction = 0.2f;

            ptr->CreateFixture(&fixture_);

            ptr->SetLinearDamping(0.4f); // more completely invented numbers..
            ptr->SetAngularDamping(0.5f);
            ptr->SetSleepingAllowed(true);

            ptr->ApplyLinearImpulse(b2Vec2{std::cos(o.angle_ / si::radians), std::sin(o.angle_ / si::radians)}, ptr->GetLocalCenter(), true);

            return ptr;
        }

        b2Body* operator()(gtl::physics::commands::destroy_object_implode const& o) const
        {

            auto range = entity_map_.equal_range(o.id_);
            if (range.first == range.second)
            {
                return nullptr;
            }

            b2AABB body_aabb_{b2Vec2{FLT_MAX, FLT_MAX}, b2Vec2{-FLT_MAX, -FLT_MAX}};

            for (auto it = range.first; it != range.second; ++it)
            {
                b2Body* body_ptr_ = it->second;
                b2Fixture* fixture_ = body_ptr_->GetFixtureList();
                while (fixture_)
                {
                    body_aabb_.Combine(body_aabb_, fixture_->GetAABB(0)); // check this index for "chained" fixtures..
                    fixture_ = fixture_->GetNext();
                }
                body_aabb_.lowerBound -= b2Vec2{12.0f, 12.0f};
                body_aabb_.upperBound += b2Vec2{12.0f, 12.0f};
            }

            auto implode_around_AABB = make_query_callback([&](b2Fixture* fixture_) {
                b2Body* b = fixture_->GetBody();
                b->SetAwake(true);
                b2Vec2 vec_{body_aabb_.GetCenter() - b->GetWorldCenter()};
                float mag_ = vec_.Normalize();
                vec_ *= ((1000.0f * b->GetMass()) / mag_);
                b->ApplyForceToCenter(vec_, true);
                return true; // continue the query
            });

            for (auto it = range.first; it != range.second; ++it)
            {
                world_.DestroyBody(it->second);
            }

            entity_map_.erase(o.id_);

            world_.QueryAABB(&implode_around_AABB, body_aabb_);

            return nullptr;
        }

        b2Body* operator()(gtl::physics::commands::boost_object const& o) const
        {

            auto range = entity_map_.equal_range(o.id_);
            if (range.first == range.second)
            {
                return nullptr;
            }

            b2Body* main_body_ = (range.first)->second;
            main_body_->SetAwake(true);

            // main_body_->Set

            main_body_->ApplyLinearImpulse(b2Vec2{100.0f * std::cos(-1.0f * main_body_->GetAngle()), 100.0f * std::sin(-1.0f * main_body_->GetAngle())},
                                           main_body_->GetWorldCenter(), true);
            // main_body_->GetPosition(), true);
            return nullptr;
        }

        b2Body* operator()(gtl::physics::commands::boost_object_vec const& o) const
        {

            auto range = entity_map_.equal_range(o.id_);
            if (range.first == range.second)
            {
                return nullptr;
            }

            b2Body* main_body_ = (range.first)->second;
            main_body_->SetAwake(true);

            // main_body_->ApplyForceToCenter(b2Vec2{o.x, o.y}, true);
            main_body_->ApplyForceToCenter(b2Vec2{o.x * 200.0f, o.y * 200.0f}, true);
            // main_body_->ApplyLinearImpulse(b2Vec2{o.x * 10.0f, o.y * 10.0f}, main_body_->GetWorldCenter(), true);

            // main_body_->GetPosition(), true);
            return nullptr;
        }

        b2Body* operator()(gtl::physics::commands::drive_object_vec const& o) const
        {

            auto range = entity_map_.equal_range(o.id_);
            if (range.first == range.second)
            {
                return nullptr;
            }

            b2Body* main_body_ = (range.first)->second;
            main_body_->SetAwake(true);

            main_body_->ApplyForce(b2Vec2{o.x, o.y}, main_body_->GetWorldCenter(), true);

            // main_body_->ApplyLinearImpulse(b2Vec2{o.x,o.y},// o.x / 100.0f,o.y / 100.0f},
            //                                      main_body_->GetWorldCenter(), true);
            // main_body_->GetPosition(), true);
            return nullptr;
        }
    };
}

//#############################################################################

namespace physics {

    simulation_dummy_impl::simulation_dummy_impl(vn::single_consumer_queue<gtl::physics::command_variant>& tasks_,
                                                 gtl::draw_kit& b2d_adapter_) // boost::units::quantity<boost::units::si::area> area_,
    // std::vector<Eigen::Vector4f> positions_)

    {
        quit_.test_and_set();
        // thread_ =
        // std::thread{&simulation_thread<render_data,gtl::physics::generator>,std::ref(entities_),std::ref(tasks_),std::ref(quit_)};
        thread_ = std::thread{&bullet_simulation_thread<gtl::physics::simulation_render_data, gtl::physics::command_variant>, std::ref(render_data_),
                              std::ref(tasks_), std::ref(quit_), std::ref(b2d_adapter_)};
    }
}

//#############################################################################

namespace {

    template <typename Get, typename Set>
    struct motion_state_helper : btMotionState, Get, Set {

        btTransform default_transform_;

        template <typename G, typename S, typename... Ts>
        motion_state_helper(G&& get, S&& set, Ts&&... ts) : Get(std::forward<G>(get)), Set(std::forward<S>(set)), default_transform_(std::forward<Ts>(ts)...)
        {
        }

        virtual void getWorldTransform(btTransform& worldTrans) const final
        {
            (Get::operator())(worldTrans, default_transform_);
        }

        virtual void setWorldTransform(btTransform const& worldTrans) final
        {
            (Set::operator())(worldTrans);
        }

        ~motion_state_helper() final
        {
        }
    };

    template <typename Get, typename Set, typename... Ts>
    auto make_motion_state_get_set(Get&& get, Set&& set, Ts&&... ts)
    {
        return motion_state_helper<std::remove_reference_t<Get>, std::remove_reference_t<Set>>{std::forward<Get>(get), std::forward<Set>(set),
                                                                                               std::forward<Ts>(ts)...};
    }

    class BulletEntity : public btMotionState {
        btTransform def_transform_;
        btScalar mass_;
        btVector3 fall_inertia_;
        btSphereShape shape_;

        Eigen::Matrix4f my_transform_;
        gtl::entity::render_data entity_data_;

        gtl::physics::simulation_render_data& render_data_;
        // std::vector<gtl::physics::gtl::entity::render_data>& entity_vector_;
        // std::vector<Eigen::Matrix4f>& render_vector_;

    public:
        BulletEntity(btTransform transform, btScalar mass, btVector3 fall_inertia, btScalar shaperadius, gtl::entity::render_data entity_render_data,
                     gtl::physics::simulation_render_data& rdata)
            : def_transform_{transform},
              mass_{mass},
              fall_inertia_{fall_inertia},
              shape_{shaperadius},
              my_transform_{},
              entity_data_{entity_render_data},
              render_data_{rdata}
        {
            setWorldTransform(transform);
        }

        btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo()
        {
            return {mass_, this, &shape_, fall_inertia_};
        }

        // BulletEntity(BulletEntity&&) = default;
        // BulletEntity& operator=(BulletEntity&&) = delete;

        ~BulletEntity() final
        {
        }

        virtual void getWorldTransform(btTransform& worldTrans) const final
        {
            worldTrans = def_transform_;
        }

        virtual void setWorldTransform(btTransform const& worldTrans) final
        {

            const auto z_plane = Eigen::Vector3f{0.0f, 0.0f, 1.0f};

            // HACK specify plane (0.1f) elsewhere..
            auto rot = worldTrans.getRotation();
            my_transform_
                = (Eigen::Affine3f{Eigen::Translation3f{Eigen::Vector3f{worldTrans.getOrigin().x(), worldTrans.getOrigin().y(), worldTrans.getOrigin().z()}}}
                   * Eigen::Affine3f{Eigen::Quaternionf{rot.x(), rot.y(), rot.z(), rot.w()}})
                      .matrix(); //.transpose();
        }

        void render()
        {
            render_data_.entities_.emplace_back(entity_data_);
            render_data_.control_points_.emplace_back(my_transform_);
        }
    };

    //#############################################################################

    template <typename T, typename U>
    static void bullet_simulation_thread(vn::swap_object<T>& rend_data_, vn::single_consumer_queue<U>& physics_task_queue_, std::atomic_flag& quit_,
                                         gtl::draw_kit& b2d_adapter_)
    {
        gtl::draw_data local_debug_render_data_;
        gtl::physics::simulation_render_data local_render_data_;

        auto add_rect_at = [&](auto x, auto y, auto h) {

            //                std::pair<float,float> coords[]={{x-h,y-h},{x+h,y-h},{x+h,y+h},{x-h,y+h}};
            //
            //                for (auto&& e : coords) {
            //                    local_debug_render_data_.add_vertex(ImDrawVert{{e.first,e.second},{0.0f,0.0f},ImColor{1.0f,1.0f,1.0f,1.0f}});
            //                }
            //
            //                local_debug_render_data_.add_triangle(3,0,1);
            //                local_debug_render_data_.add_triangle(3,1,2);
            //
            //                local_debug_render_data_.next_group();
            //
        };

        btDbvtBroadphase broadphase{};
        btDefaultCollisionConfiguration collisionConfiguration{};
        btCollisionDispatcher dispatcher{&collisionConfiguration};
        btSequentialImpulseConstraintSolver solver{};
        btDiscreteDynamicsWorld dynamicsWorld{&dispatcher, &broadphase, &solver, &collisionConfiguration};

        dynamicsWorld.setGravity(btVector3{0.0f, -2.0f, 0.0f});

        btStaticPlaneShape groundShape{btVector3(0.0f, 1.0f, 0.0f), 1.0f};
        btSphereShape fallShape{1.0f};

        btDefaultMotionState groundMotionState{btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, -1.0f, 0.0f))};
        btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI{0, &groundMotionState, &groundShape, btVector3(0.0f, 0.0f, 10.0f)};

        btRigidBody groundRigidBody{groundRigidBodyCI};

        dynamicsWorld.addRigidBody(&groundRigidBody);

        auto fallMotionState = make_motion_state_get_set(
            [&](auto& wt, auto const& default_transform) {
                // updates wt
                wt = default_transform;
            },
            [&](auto const& wt) {
                // wt reports
                add_rect_at(wt.getOrigin().getX(), -wt.getOrigin().getY(), 2.0f);
            },
            btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, 10.0f, 0.0f)));

        // btDefaultMotionState fallMotionState{btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, 10.0f, 0.0f))};

        btScalar mass = 1.0f;
        btVector3 fallInertia(0.0f, 0.0f, 0.0f);
        fallShape.calculateLocalInertia(mass, fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, &fallMotionState, &fallShape, fallInertia);
        btRigidBody fallRigidBody{fallRigidBodyCI};
        dynamicsWorld.addRigidBody(&fallRigidBody);

        //##############################
        std::vector<BulletEntity> entities_;

        for (int i = 0; i < 100; ++i)
        {
            entities_.emplace_back(
                btTransform{btQuaternion{0.0f, 0.0f, 0.0f, 1.0f},
                            btVector3{vn::math::rand_neg_one_one() * 10.0f, vn::math::rand_zero_one() * 20.0f, vn::math::rand_neg_one_one() * 10.0f}},
                1.0f, btVector3{0.0f, vn::math::rand_zero_one() * 1.0f, 0.0f}, 1.0f,
                gtl::entity::render_data{}.pack_entity_id(i + 55).pack_mesh_id(1).pack_bone_offset(static_cast<uint16_t>(i)), local_render_data_);
        }
        //#############################

        std::vector<btRigidBody> bodies_;
        bodies_.reserve(200);

        for (auto&& e : entities_)
        {
            bodies_.emplace_back(e.rigidBodyInfo());
            bodies_.back().setRestitution(1.2f);
            bodies_.back().setDamping(0.8f, 0.2f);
        }

        for (auto&& b : bodies_)
        {
            dynamicsWorld.addRigidBody(&b);
        }

        gtl::rate_limiter rate_limiter_{std::chrono::milliseconds(15)};

        while (quit_.test_and_set(std::memory_order_acquire))
        {
            rate_limiter_.sleepy_invoke([&](auto dt) {

                local_debug_render_data_.clear();
                local_render_data_.clear();

                dynamicsWorld.stepSimulation(std::chrono::duration<float>(dt).count(), 4);

                // http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Stepping_The_World

                b2d_adapter_.render(local_debug_render_data_);

                for (auto&& e : entities_)
                {
                    e.render();
                }

                rend_data_.swap_in(local_render_data_);
            });
        }

        for (auto&& b : bodies_)
        {
            dynamicsWorld.removeRigidBody(&b);
        }

        dynamicsWorld.removeRigidBody(&fallRigidBody);
        dynamicsWorld.removeRigidBody(&groundRigidBody);
    }

    //#############################################################################

    template <typename T, typename U>
    static void simulation_thread(vn::swap_object<T>& rend_data_,
                                  vn::single_consumer_queue<U>& physics_task_queue_, // gtl::swap_vector<U>& physics_task_queue_,
                                  // std::vector<physics::generator> generators_,
                                  std::atomic_flag& quit_, gtl::draw_kit& b2d_adapter_)
    {
        // b2World world_{b2Vec2{0.0f,-9.8f}};
        b2World world_{b2Vec2{0.0f, 0.0f}};
        map_type map_;

        world_.SetAllowSleeping(true);

        box2d_generator_visitor visitor_{world_, map_};

        // auto task_local_ = physics_task_queue_.make_vector();
        // physics_task_queue_.swap_out(task_local_);

        physics_task_queue_.consume([&](auto&& e) { boost::apply_visitor(visitor_, e); });

        // for (auto&& e : task_local_)
        //{
        //    apply_visitor(visitor_, e);
        //}

        float32 timeStep = 1.0f / 60.0f;
        int32 velocityIterations = 8;
        int32 positionIterations = 3;

        render_data local_rend_data_;

        gtl::draw_data local_b2d_data_;

        auto& positions_ = local_rend_data_.entities_;
        auto& bones_ = local_rend_data_.bones_;

        // std::vector<T> positions_;
        // positions_.reserve(100);

        std::vector<b2Body*> selected_bodies_;
        selected_bodies_.reserve(100);

        int64_t max_wait_ = 0;

        auto query_bodies = [&]() {
            // positions_.clear();
            for (b2Body* b = world_.GetBodyList(); b; b = b->GetNext())
            {
                b2Vec2 position = b->GetPosition();
                // uint32_t index_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b->GetUserData()));
                // temporar

                // positions_.emplace_back(gtl::entity::render_data{{position.x,
                //                           position.y,
                //                           1.0f,1.0f},
                //                           0, // MESH_ID
                //                           index_});
            }
            // swapvec_.swap_in(positions_);
        };

        auto query_around_AABB = make_query_callback([&](b2Fixture* fixture_) {
            // b2Body *b = fixture_->GetBody();
            // b->SetAwake(true);
            // b2Vec2 position = b->GetPosition();
            // uint32_t index_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b->GetUserData()));
            //
            ////b2AABB aabb_ = fixture_->GetAABB(0);
            //
            // positions_.emplace_back(gtl::entity::render_data{{position.x,
            //                           position.y,
            //                           1.0f,1.0f},
            //                           {1.0f, 1.0f, 1.0f, b->GetAngle()},index_});
            ////positions_.emplace_back(T{{position.x,
            ////                           position.y,
            ////                           aabb_.GetExtents().x,aabb_.GetExtents().y},
            ////                           {1.0f, 1.0f, 1.0f, 0.0f},index_});
            // return true; // continue the query

            b2Body& body_ = *(fixture_->GetBody());
            body_.SetAwake(true);

            // uint32_t index_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(body_.GetUserData()));
            b2Vec2 const& p = body_.GetPosition();

            // positions_.emplace_back(gtl::entity::render_data{{p.x, p.y, 1.0f, 1.0f},
            //                                   //{1.0f, 1.0f, 1.0f, body_.GetAngle()},
            //                                   0, // MESH_ID
            //                                   static_cast<uint32_t>(bones_.size()),
            //                                   index_});
            // single body addition..
            Eigen::Matrix4f m = (Eigen::Affine3f{Eigen::Translation3f{Eigen::Vector3f{p.x, p.y, 0.1f}}} // HACK specify plane (0.1f) elsewhere..
                                 * Eigen::Affine3f{Eigen::AngleAxisf{body_.GetAngle(), Eigen::Vector3f{0.0f, 0.0f, 1.0f}}})
                                    .matrix(); // Eigen::Rotation2Df{body_.GetAngle()}.matrix();

            bones_.emplace_back(m.transpose());

            return true;
        });

        auto actually_dump_bodies = [&](b2Body& body_) {
            auto* fixture_ = body_.GetFixtureList();
            while (fixture_)
            {
                auto type = fixture_->GetType();
                switch (type)
                {
                    case b2Shape::Type::e_polygon:
                    {
                        auto* p = static_cast<b2PolygonShape*>(fixture_->GetShape());

                        auto v_count = p->GetVertexCount();

                        for (int i = 0; i < v_count; ++i)
                        {
                            // auto v = p->GetVertex(i);
                            auto v = body_.GetWorldPoint(p->GetVertex(i));
                            local_b2d_data_.add_vertex(ImDrawVert{{v.x, -v.y}, {0.0f, 0.0f}, ImColor{1.0f, 1.0f, 1.0f, 1.0f}});
                        }

                        for (int i = 0; i < v_count - 2; ++i)
                        {
                            local_b2d_data_.add_triangle(v_count - 1, i, i + 1);
                        }

                        local_b2d_data_.next_group();
                    }
                    default:
                        break;
                }
                fixture_ = fixture_->GetNext();
            }
        };

        auto dump_debug_fixtures = [&](b2Fixture& fixture_) {
            auto type = fixture_.GetType();
            switch (type)
            {
                case b2Shape::Type::e_polygon:
                {
                    auto* p = static_cast<b2PolygonShape*>(fixture_.GetShape());
                    auto v_count = p->GetVertexCount();
                    for (int i = 0; i < v_count; ++i)
                    {
                        auto v = fixture_.GetBody()->GetWorldPoint(p->GetVertex(i));
                        local_b2d_data_.add_vertex(ImDrawVert{{v.x, -v.y}, {0.0f, 0.0f}, ImColor{1.0f, 1.0f, 1.0f, 1.0f}});
                    }

                    for (int i = 0; i < v_count - 2; ++i)
                    {
                        local_b2d_data_.add_triangle(v_count - 1, i, i + 1);
                    }

                    local_b2d_data_.next_group();
                }
                default:
                    break;
            }
        };

        auto dump_fixtures = [&](b2Body& body_) {
            gtl::entity::render_data instance{reinterpret_cast<uintptr_t>(body_.GetUserData()), static_cast<uint16_t>(bones_.size())};

            b2Vec2 const& p = body_.GetPosition();

            positions_.emplace_back(instance);

            // process all bodies that participate in the entity..
            auto range = map_.equal_range(instance.entity_id());
            for (auto it = range.first; it != range.second; ++it)
            {
                b2Vec2 const& p = it->second->GetPosition();
                float const& angle = it->second->GetAngle();

                Eigen::Matrix4f m = (Eigen::Affine3f{Eigen::Translation3f{Eigen::Vector3f{p.x, p.y, 0.1f}}}
                                     * Eigen::Affine3f{Eigen::AngleAxisf{angle, Eigen::Vector3f{0.0f, 0.0f, 1.0f}}})
                                        .matrix();

                bones_.emplace_back(m.transpose());
            }
        };

        auto query_bodies_around_AABB = make_query_callback([&](b2Fixture* fixture_) {
            b2Body* const body_ptr_ = reinterpret_cast<b2Body*>(fixture_->GetUserData());

            body_ptr_->SetAwake(true);
            selected_bodies_.emplace_back(body_ptr_);

            dump_debug_fixtures(*fixture_);

            return true; // continue the query
        });

        b2AABB aabb;
        aabb.lowerBound.Set(-60.0f, -60.0f);
        aabb.upperBound.Set(60.0f, 60.0f);

        gtl::rate_limiter rate_limiter_{std::chrono::milliseconds(15)};

        while (quit_.test_and_set(std::memory_order_acquire))
        {
            rate_limiter_.sleepy_invoke([&](auto dt) {
                bones_.clear();
                positions_.clear();
                selected_bodies_.clear();
                local_b2d_data_.clear();

                world_.QueryAABB(&query_bodies_around_AABB, aabb);

                std::sort(begin(selected_bodies_), end(selected_bodies_));
                auto new_end_ = std::unique(begin(selected_bodies_), end(selected_bodies_));
                selected_bodies_.erase(new_end_, end(selected_bodies_));

                for (auto&& e : selected_bodies_)
                {
                    dump_fixtures(*e);
                }

                rend_data_.swap_in(local_rend_data_);
                b2d_adapter_.render(local_b2d_data_);

                physics_task_queue_.consume([&](auto&& e) { boost::apply_visitor(visitor_, e); });

                world_.Step(std::chrono::duration<float>(dt).count(), velocityIterations, positionIterations);
            });
        }

        std::cout << "physics thread exiting, max wait == " << max_wait_ << "\n";
    }

} // namespace
} // namespace

/*
class MyContactListener : public b2ContactListener
  {
    void BeginContact(b2Contact* contact) {

      //check if fixture A was a ball
      void* bodyUserData = contact->GetFixtureA()->GetBody()->GetUserData();
      if ( bodyUserData )
        static_cast<Ball*>( bodyUserData )->startContact();

      //check if fixture B was a ball
      bodyUserData = contact->GetFixtureB()->GetBody()->GetUserData();
      if ( bodyUserData )
        static_cast<Ball*>( bodyUserData )->startContact();

    }

    void EndContact(b2Contact* contact) {

      //check if fixture A was a ball
      void* bodyUserData = contact->GetFixtureA()->GetBody()->GetUserData();
      if ( bodyUserData )
        static_cast<Ball*>( bodyUserData )->endContact();

      //check if fixture B was a ball
      bodyUserData = contact->GetFixtureB()->GetBody()->GetUserData();
      if ( bodyUserData )
        static_cast<Ball*>( bodyUserData )->endContact();

    }
  };
*/