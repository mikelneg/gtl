#include "gtl/physics_simulation.h"

#include <gtl/swap_vector.h>

#include <Box2D/Box2D.h>
#include <vector>
#include <iostream>
#include <thread>
#include <memory>
#include <atomic>
#include <limits>
#include <functional>
#include <unordered_map>

#include <cstdint>
#include <cmath>
#include <cassert>

#include <Eigen/Core>

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
            ENTITY   = 2,
            SENSOR   = 4
        };
    };    

    template <typename T, typename U>
    static void simulation_thread(gtl::swap_vector<T>& swapvec_, 
                              gtl::swap_vector<U>& task_queue_,
                              std::atomic_flag& quit_);


    template <typename T>
    struct query_callback_helper : b2QueryCallback, T {
        query_callback_helper(T t) : T(t) {}
        bool ReportFixture(b2Fixture* fixture_) override {
            return (T::operator())(fixture_);
        }        
    };

    template <typename T>
    auto make_query_callback(T t) { return query_callback_helper<T>{std::move(t)}; }


    struct box2d_generator_visitor : boost::static_visitor<b2Body*> {

        b2World& world_;

        std::unordered_map<uint32_t,b2Body*>& entity_map_;

        box2d_generator_visitor(b2World& w_, std::unordered_map<uint32_t,b2Body*>& map_) noexcept : world_{w_}, entity_map_{map_} {}

        template <typename T>
        b2Body* operator()(T const& t) const { assert(false); return nullptr; }

        b2Body* operator()(gtl::physics::generators::static_box const& o) const {        
            
            using namespace boost::units;
            
            b2BodyDef body_;

            body_.position = b2Vec2{o.xy_.first / si::meters, o.xy_.second / si::meters};
            body_.angle = o.angle_ / si::radians; 
            body_.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(o.id)); 
            body_.type = b2_staticBody;                        
            //body_.awake = true; // default
            // etc..

            b2Body* ptr = world_.CreateBody(&body_);
    
            b2PolygonShape shape_;
            shape_.SetAsBox(o.wh_.first / si::meters, o.wh_.second / si::meters);
            //shape_.SetAsBox(o.wh_.first.value(),o.wh_.second.value());//,b2Vec2{o.x,o.y},o.a);

            b2FixtureDef fixture_;

            fixture_.filter.categoryBits = collision_category::BOUNDARY;
            fixture_.filter.maskBits = collision_category::ENTITY;
            fixture_.shape = &shape_;

            ptr->CreateFixture(&fixture_);

            return ptr;            
        }

        b2Body* operator()(gtl::physics::generators::dynamic_box const& o) const {        
            
            using namespace boost::units;
            
            b2BodyDef body_;

            body_.position = b2Vec2{o.xy_.first / si::meters, o.xy_.second / si::meters};
            body_.angle = o.angle_ / si::radians;
            body_.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(o.id)); 
            body_.type = b2_dynamicBody;             
            //body_.awake = true; // default
            // etc..

            b2Body* ptr = world_.CreateBody(&body_);            

            entity_map_.emplace(o.id, ptr);

            b2PolygonShape shape_;
            shape_.SetAsBox(o.wh_.first / si::meters, o.wh_.second / si::meters);
            //shape_.SetAsBox(o.wh_.first.value(),o.wh_.second.value());//,b2Vec2{o.x,o.y},o.a);

            b2FixtureDef fixture_;

            fixture_.filter.categoryBits = collision_category::ENTITY;
            fixture_.filter.maskBits = collision_category::ENTITY | 
                                       collision_category::SENSOR | 
                                       collision_category::BOUNDARY;
            fixture_.shape = &shape_;
            
            fixture_.isSensor = false;
            fixture_.restitution = 0.46f;
            fixture_.density = 1.0f;  // default numbers from the docs, revisit later..
            fixture_.friction = 0.3f; 
                       
            ptr->CreateFixture(&fixture_);
                        
            ptr->SetLinearDamping(0.4f); // more completely invented numbers..
            ptr->SetAngularDamping(0.5f);            
            ptr->SetSleepingAllowed(true);        

            ptr->ApplyLinearImpulse(b2Vec2{std::cos(o.angle_ / si::radians),
                                           std::sin(o.angle_ / si::radians)},ptr->GetLocalCenter(), true);

            return ptr;            
        }

        b2Body* operator()(gtl::physics::generators::destroy_object_implode const& o) const {
            
            auto it = entity_map_.find(o.id);
            if (it == entity_map_.end()) { return nullptr; }
            
            b2Body* body_ptr_ = it->second;

            b2AABB body_aabb_;
            body_aabb_.lowerBound = b2Vec2(FLT_MAX,FLT_MAX);
            body_aabb_.upperBound = b2Vec2(-FLT_MAX,-FLT_MAX); 
            
            b2Fixture* fixture_ = body_ptr_->GetFixtureList();
            while (fixture_)
            {
                body_aabb_.Combine(body_aabb_, fixture_->GetAABB(0)); // check this index for "chained" fixtures..
                fixture_ = fixture_->GetNext();
            }

            body_aabb_.lowerBound -= b2Vec2{12.0f,12.0f};
            body_aabb_.upperBound += b2Vec2{12.0f,12.0f};            

            auto implode_around_AABB = make_query_callback(
                [&](b2Fixture* fixture_){                 
                    b2Body *b = fixture_->GetBody();
                    //b->SetAwake(true);                        
                    b2Vec2 vec_{body_aabb_.GetCenter() - b->GetWorldCenter()};
                    float mag_ = vec_.Normalize();
                    
                    vec_ *= ((1000.0f * body_ptr_->GetMass()) / mag_);// * body_ptr_->GetMass();                    
                    
                    //b->ApplyLinearImpulse(vec_, b->GetLocalCenter(),true);
                    b->ApplyForceToCenter(vec_,true);
                    //b->ApplyForce(vec_, b->GetLocalCenter(),true);
                    return true; // continue the query
            });

            world_.QueryAABB(&implode_around_AABB, body_aabb_);

            world_.DestroyBody(body_ptr_);
            entity_map_.erase(o.id);

            return nullptr;
        }

    };

}

physics_simulation::physics_simulation(gtl::swap_vector<gtl::physics::generator>& tasks_)//boost::units::quantity<boost::units::si::area> area_,
                                       //std::vector<Eigen::Vector4f> positions_)  

{
    quit_.test_and_set();    
    thread_ = std::thread{&simulation_thread<entity_type,gtl::physics::generator>,std::ref(entities_),std::ref(tasks_),std::ref(quit_)};
}

namespace {

template <typename T, typename U>
static void simulation_thread(gtl::swap_vector<T>& swapvec_, 
                              gtl::swap_vector<U>& task_queue_,
                              //std::vector<physics::generator> generators_,
                              std::atomic_flag& quit_) 
{    
    //b2World world_{b2Vec2{0.0f,-9.8f}};  
    b2World world_{b2Vec2{0.0f,0.0f}};  
    std::unordered_map<uint32_t,b2Body*> map_;

    world_.SetAllowSleeping(true);
       
    box2d_generator_visitor visitor_{world_,map_};

    auto task_local_ = task_queue_.make_vector();
    task_queue_.swap_out(task_local_);

    for (auto&& e : task_local_) {
        apply_visitor(visitor_,e);
    }
    
    float32 timeStep = 1.0f / 60.0f;
    int32 velocityIterations = 8;
    int32 positionIterations = 3;

    std::vector<T> positions_; 
    positions_.reserve(100);

    int64_t max_wait_ = 0;

    auto query_bodies = [&](){     
        //positions_.clear();
        for (b2Body* b = world_.GetBodyList(); b; b = b->GetNext())
        {                    
            b2Vec2 position = b->GetPosition();            
            uint32_t index_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b->GetUserData()));
            // temporary
            positions_.emplace_back(T{{position.x / 120.0f,
                                       position.y / 120.0f,
                                       0.01f,0.01f},
                                      {1.0f, 1.0f, 1.0f, b->GetAngle()},index_});            
        }
        //swapvec_.swap_in(positions_);
    };

    
    auto query_around_AABB = make_query_callback(
        [&](b2Fixture* fixture_){                 
            b2Body *b = fixture_->GetBody();
            b->SetAwake(true);                        
            b2Vec2 position = b->GetPosition();                           
            uint32_t index_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b->GetUserData()));
            positions_.emplace_back(T{{position.x,
                                       position.y,
                                       0.01f,0.01f},
                                       {1.0f, 1.0f, 1.0f, b->GetAngle()},index_});                                    
            return true; // continue the query
    });
    
    b2AABB aabb;
    aabb.lowerBound.Set(-60.0f, -60.0f);
    aabb.upperBound.Set(60.0f, 60.0f);    

    auto time_ = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(15);          

    while (quit_.test_and_set(std::memory_order_acquire)) {
        
        positions_.clear();
        world_.QueryAABB(&query_around_AABB, aabb);
        //query_bodies();
        swapvec_.swap_in(positions_);

        task_local_.clear();
        task_queue_.swap_out(task_local_);
        for (auto&& e : task_local_) {
            apply_visitor(visitor_,e);
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


/*


static void simulation_thread(gtl::swap_vector<Eigen::Vector4f>& swapvec_, 
                              std::vector<physics::generator> generators_,
                              std::atomic_flag& quit_) 
{    
    b2World world_{b2Vec2{0.0f,-0.1f}};  
    world_.SetAllowSleeping(true);
    
    
    box2d_generator_visitor visitor_{world_};
    for (auto&& e : generators_) {
        apply_visitor(visitor_,e);
    }
    
    //std::vector<b2BodyDef> wall_defs_; // ground_, ceiling_, left_wall_, right_wall_;
    //
    //wall_defs_.emplace_back(b2Vec2{0.0f,-25.0f},0.0f);
    //wall_defs_.emplace_back(b2Vec2{0.0f,25.0f},0.0f);
    //wall_defs_.emplace_back(b2Vec2{-25.0f,0.0f},0.0f);
    //wall_defs_.emplace_back(b2Vec2{25.0f,0.0f},0.0f);
    //
    //std::vector<b2Body*> wall_bodies_;
    //
    //for (auto&& e : wall_defs_) {
    //    wall_bodies_.emplace_back(world_.CreateBody(&e));
    //    wall_bodies_.back()->SetUserData(reinterpret_cast<void*>(std::numeric_limits<uintptr_t>::max())); 
    //}
    //
    ////b2Body* groundBody_ = world_.CreateBody(&ground_);
    ////b2Body* ceilb_ = world_.CreateBody(&ceiling_);
    ////b2Body* leftb_ = world_.CreateBody(&left_wall_);
    ////b2Body* rightb_ = world_.CreateBody(&right_wall_);
    //
    //std::vector<b2PolygonShape> wall_shapes_;
    //
    //wall_shapes_.emplace_back();
    //wall_shapes_.back().SetAsBox(60.0f, 1.0f);
    //
    //wall_shapes_.emplace_back();
    //wall_shapes_.back().SetAsBox(60.0f, 1.0f);
    //
    //wall_shapes_.emplace_back();
    //wall_shapes_.back().SetAsBox(1.0f, 60.0f);
    //
    //wall_shapes_.emplace_back();
    //wall_shapes_.back().SetAsBox(1.0f, 60.0f);
    //
    //std::vector<b2FixtureDef> wall_fixtures_;
    //
    ////b2FixtureDef grounddef_{}, ceildef_{}, leftdef_{}, rightdef_{};
    //
    //for (auto&& e : wall_defs_) {
    //    wall_fixtures_.emplace_back();
    //    wall_fixtures_.back().filter.categoryBits = CollisionCategory::BOUNDARY;
    //    wall_fixtures_.back().filter.maskBits = CollisionCategory::ENTITY;
    //}
    //
    //for (unsigned i = 0; i < wall_shapes_.size(); ++i) {
    //    wall_fixtures_[i].shape = &wall_shapes_[i];
    //    wall_bodies_[i]->CreateFixture(&wall_fixtures_[i]);
    //}               
    //
    //std::vector<b2Body*> bodies_;
    //
    //for (uintptr_t i = 0; i < positions_.size(); ++i) {
    //    b2BodyDef body_;        
    //    body_.type = b2_dynamicBody;
    //    body_.position.Set(positions_[i].x() * 500.0f, positions_[i].y() * 500.0f); // TODO where did I get these??
    //    body_.userData = reinterpret_cast<void*>(i); 
    //    bodies_.emplace_back(world_.CreateBody(&body_));    
    //
    //    // // sensor stuff
    //    //b2PolygonShape coneShape;
    //    //b2FixtureDef coneFixture;
    //    //
    //    //std::vector<b2Vec2> cone_shape_ = get_cone_shape({0,0}, 4.0f, 10.0f, 4);
    //    //
    //    ////sensor_data_[i].x() = 1.0f; // distance // currently just carries color 
    //    ////sensor_data_[i].y() = 30.0f; // angle
    //    //
    //    //coneShape.Set(cone_shape_.data(), static_cast<int32>(cone_shape_.size()));        
    //    //
    //    //
    //    //b2CircleShape circleShape{};
    //    //circleShape.m_radius = 3;
    //    //
    //    //coneFixture.shape = &coneShape;
    //    //
    //    //coneFixture.isSensor = true;
    //    ////coneFixture.density = 0.0f;
    //    //
    //    ////coneFixture.filter.groupIndex = 1;
    //    //coneFixture.filter.categoryBits = CollisionCategory::SENSOR;
    //    //coneFixture.filter.maskBits = CollisionCategory::ENTITY;
    //
    //    // Bounding Box
    //
    //    b2PolygonShape bodyShape;
    //    b2FixtureDef bodyFixture; 
    //
    //    bodyShape.SetAsBox(0.2f,0.2f);    // 0.002f scale 
    //    bodyFixture.shape = &bodyShape;
    //    bodyFixture.density = 2.0f;
    //    bodyFixture.friction = 0.1f;
    //    
    //    bodyFixture.isSensor = false;
    //    //bodyFixture.filter.groupIndex = 2;
    //    bodyFixture.filter.categoryBits = CollisionCategory::ENTITY;
    //    bodyFixture.filter.maskBits = CollisionCategory::ENTITY | CollisionCategory::SENSOR | CollisionCategory::BOUNDARY;
    //
    //    //if (i == 0) {
    //    //    bodyPtr->CreateFixture(&coneFixture);
    //    //    }
    //    b2Body& b = *bodies_.back();
    //    b.SetSleepingAllowed(true);
    //    b.CreateFixture(&bodyFixture);                
    //    b.SetLinearDamping(0.001f); // both were 2.0f
    //    b.SetAngularDamping(0.001f);
    //} 

    float32 timeStep = 1.0f / 60.0f;
    int32 velocityIterations = 8;
    int32 positionIterations = 3;

    //shared_data_.get_writer("sensors").write<game_utils::shared_types::positions>(sensors_);

    //write(eds.sensors(),std::begin(sensor_data_),sensor_data_.size());       


    std::vector<Eigen::Vector4f> positions_; positions_.reserve(100);

    auto time_ = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(15);
    
    while (quit_.test_and_set(std::memory_order_acquire)) {

        positions_.clear();

        for (b2Body* b = world_.GetBodyList(); b; b = b->GetNext())
        {                    
            b2Vec2 position = b->GetPosition();
            float angle = b->GetAngle();
            
            uintptr_t index_ = reinterpret_cast<uintptr_t>(b->GetUserData());

            //if (index_ < std::numeric_limits<uintptr_t>::max()) {
             //   positions_[index_].x() = position.x / 50.0f;
             //   positions_[index_].y() = position.y / 50.0f;
            //}
            positions_.emplace_back(Eigen::Vector4f{position.x / 70.0f,
                                                    position.y / 70.0f,
                                                    0.02f,0.02f});            
        }

        swapvec_.swap_in(positions_);

        world_.Step(timeStep, velocityIterations, positionIterations);
                
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
        //float32 angle = body->GetAngle();
 
        //--for (auto& e : orientation_data_) {
        //--  g3d::Quaternionf object_rotation_ = 
        //--    g3d::Quaternionf::FromTwoVectors(g3d::Vector3f{0.0001f,0.0001f,1.000001f}.normalized(),
        //--                                     g3d::Vector3f{0.0f,0.0f,1.0f}.normalized());
        //--
        //--    e *= object_rotation_;
        //--    e.normalize();
        //--
        //--    //e.position_.z() -= 0.00001f;
        //--    //e.position_.y += game_utils::rand_neg_one_one();
        //--    //e.position_.z += game_utils::rand_neg_one_one();
        //--}
        //blah_ = working_data_;
        //push(working_data_,entity_data_);

        auto now_ = std::chrono::high_resolution_clock::now();
        if (now_ < time_) {            
            std::this_thread::sleep_for(time_ - now_);
            time_ = now_ + std::chrono::milliseconds(15);
        }
    }    
}

*/