/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#pragma warning(disable : 4503)

#include "gtl/physics/bullet/simulation_impl.h"
#include <gtl/physics/bullet/command_visitor.h>

#include <iostream>

#include <atomic>
#include <thread>

#include <vn/single_consumer_queue.h>
#include <vn/swap_object.h>

#include <gtl/physics/common_types.h>
#include <gtl/physics/command_variant.h>
#include <gtl/box2d_adapter.h>               // HACK change this..

#include <gtl/rate_limiter.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include <cassert>
#include <cmath>
#include <cstdint>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <vn/math_utilities.h>

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
    template <typename T, typename U>
    static void bullet_simulation_thread(vn::swap_object<T>& rend_data_, 
                                         vn::single_consumer_queue<U>& physics_task_queue_,        
                                         std::atomic_flag& quit_, 
                                         gtl::box2d_adapter& b2d_adapter_);    
}

namespace physics {
    bullet_simulation::bullet_simulation(vn::single_consumer_queue<gtl::physics::command_variant>& tasks_, 
                                         gtl::box2d_adapter& b2d_adapter_)
    {
        quit_.test_and_set();        
        thread_ = std::thread{
                        &bullet_simulation_thread<gtl::physics::simulation_render_data, gtl::physics::command_variant>, 
                              std::ref(render_data_),
                              std::ref(tasks_), 
                              std::ref(quit_), 
                              std::ref(b2d_adapter_)
                  };
    }
}


namespace {
    
    template <typename Get, typename Set>
    struct motion_state_helper: btMotionState, Get, Set {
        
        btTransform default_transform_;
                
        template <typename G, typename S, typename ...Ts>
        motion_state_helper(G&& get, S&& set, Ts&&...ts) 
            : Get(std::forward<G>(get)), 
              Set(std::forward<S>(set)),
              default_transform_(std::forward<Ts>(ts)...)
        {}
        
        virtual void getWorldTransform(btTransform &worldTrans) const final {
            (Get::operator())(worldTrans, default_transform_);
        }

        virtual void setWorldTransform(btTransform const& worldTrans) final {
            (Set::operator())(worldTrans);
        }
        
        ~motion_state_helper() final {}
    };

    template <typename Get, typename Set, typename ...Ts>
    auto make_motion_state_get_set(Get&& get, Set&& set, Ts&&...ts)
    {
        return motion_state_helper<std::remove_reference_t<Get>,
                                   std::remove_reference_t<Set>>
                                        {
                                            std::forward<Get>(get),
                                            std::forward<Set>(set), 
                                            std::forward<Ts>(ts)...
                                        };
    }     

    class BulletEntity : public btMotionState {        
        btTransform def_transform_;
        btScalar mass_;
        btVector3 fall_inertia_;
        btSphereShape shape_;                
        
        Eigen::Matrix4f my_transform_;
        gtl::physics::entity_render_data entity_data_;
        
        gtl::physics::simulation_render_data& render_data_;
        //std::vector<gtl::physics::gtl::physics::entity_render_data>& entity_vector_;
        //std::vector<Eigen::Matrix4f>& render_vector_;

    public:        
        BulletEntity(btTransform transform, 
                     btScalar mass, 
                     btVector3 fall_inertia,
                     btScalar shaperadius,
                     gtl::physics::entity_render_data entity_render_data,
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

        btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo() {
            return {mass_,this,&shape_,fall_inertia_};
        }

        //BulletEntity(BulletEntity&&) = default;
        //BulletEntity& operator=(BulletEntity&&) = delete;

        ~BulletEntity() final {}

        virtual void getWorldTransform(btTransform &worldTrans) const final {
            worldTrans = def_transform_;
        }

        virtual void setWorldTransform(btTransform const& worldTrans) final {
        
            const auto z_plane = Eigen::Vector3f{0.0f,0.0f,1.0f};
            
            // HACK specify plane (0.1f) elsewhere..
            auto rot = worldTrans.getRotation();
            my_transform_ = (Eigen::Affine3f{
                          Eigen::Translation3f{Eigen::Vector3f{worldTrans.getOrigin().x(), worldTrans.getOrigin().y(), worldTrans.getOrigin().z()}}}                                     
                        * Eigen::Affine3f{Eigen::Quaternionf{rot.x(),rot.y(),rot.z(),rot.w()}}).matrix().transpose();                                  
        }               

        void render() {        
            render_data_.entities_.emplace_back(entity_data_);
            render_data_.control_points_.emplace_back(my_transform_);
        }
    };


    //#############################################################################

    template <typename T, typename U>
    static void bullet_simulation_thread(
        vn::swap_object<T>& rend_data_, 
        vn::single_consumer_queue<U>& physics_task_queue_,        
        std::atomic_flag& quit_, 
        gtl::box2d_adapter& b2d_adapter_)
    {
        gtl::box2d_adapter::imgui_data local_debug_render_data_; 
        gtl::physics::simulation_render_data local_render_data_;        
                
        auto add_rect_at = 
            [&](auto x, auto y, auto h){                 

                std::pair<float,float> coords[]={{x-h,y-h},{x+h,y-h},{x+h,y+h},{x-h,y+h}};
                
                for (auto&& e : coords) {
                    local_debug_render_data_.add_vertex(ImDrawVert{{e.first,e.second},{0.0f,0.0f},ImColor{1.0f,1.0f,1.0f,1.0f}});
                }

                local_debug_render_data_.add_triangle(3,0,1);
                local_debug_render_data_.add_triangle(3,1,2);

                local_debug_render_data_.next_group();

            };

        //
        
        btDbvtBroadphase broadphase{};
        btDefaultCollisionConfiguration collisionConfiguration{};
        btCollisionDispatcher dispatcher{&collisionConfiguration};
        btSequentialImpulseConstraintSolver solver{};
        btDiscreteDynamicsWorld dynamicsWorld{&dispatcher, &broadphase, &solver, &collisionConfiguration};
        
        //

        dynamicsWorld.setGravity(btVector3{0.0f, -2.0f, 0.0f});

        btStaticPlaneShape groundShape{btVector3(0.0f, 1.0f, 0.0f), 1.0f};        
        btSphereShape fallShape{1.0f};

        btDefaultMotionState groundMotionState{btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, -1.0f, 0.0f))};
        btRigidBody::btRigidBodyConstructionInfo
            groundRigidBodyCI{0, &groundMotionState, &groundShape, btVector3(0.0f, 0.0f, 10.0f)};
        
        btRigidBody groundRigidBody{groundRigidBodyCI};
        
        dynamicsWorld.addRigidBody(&groundRigidBody);        

        // dynamicsWorld.setInternalTickCallback()
        // btIDebugDraw, implement its drawline, there you go..

        auto fallMotionState = 
            make_motion_state_get_set(
                [&](auto& wt, auto const& default_transform) {
                    // updates wt                    
                    wt = default_transform;
                }, 
                [&](auto const& wt) {
                    // wt reports 
                    add_rect_at(wt.getOrigin().getX(),-wt.getOrigin().getY(),2.0f);                
                }, 
                btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, 10.0f, 0.0f)));

        //btDefaultMotionState fallMotionState{btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, 10.0f, 0.0f))};
        
        
        btScalar mass = 1.0f;
        btVector3 fallInertia(0.0f, 0.0f, 0.0f);
        fallShape.calculateLocalInertia(mass, fallInertia);
        btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, &fallMotionState, &fallShape, fallInertia);
        btRigidBody fallRigidBody{fallRigidBodyCI};
        dynamicsWorld.addRigidBody(&fallRigidBody);        

        //##############################
        std::vector<BulletEntity> entities_;

        for (int i = 0; i < 100; ++i) {
            entities_.emplace_back( btTransform{ btQuaternion{0.0f,0.0f,0.0f,1.0f},
                                                 btVector3{vn::math::rand_neg_one_one() * 10.0f, vn::math::rand_zero_one() * 20.0f, vn::math::rand_neg_one_one() * 10.0f}},
                                    1.0f, 
                                    btVector3{0.0f,vn::math::rand_zero_one() * 1.0f,0.0f}, 
                                    1.0f, 
                                    gtl::physics::entity_render_data{}.pack_entity_id(i + 55).pack_mesh_id(1).pack_bone_offset(static_cast<uint16_t>(i)), 
                                    local_render_data_);
        }
        //#############################

        std::vector<btRigidBody> bodies_; bodies_.reserve(200);

        for (auto&& e : entities_) {
            bodies_.emplace_back(e.rigidBodyInfo());
            bodies_.back().setRestitution(1.2f);
            bodies_.back().setDamping(0.8f, 0.2f);
        }

        for (auto&& b : bodies_) {
            dynamicsWorld.addRigidBody(&b);
        }

        //CLEAN THIS UP, SWITCH TO FLAT_MAP, GET GENERATORS WORKING

        gtl::physics::command_variant v = gtl::physics::commands::boost_object{50};

        auto visitor = gtl::physics::detail::bullet_command_visitor<decltype(dynamicsWorld), decltype(mass)>{dynamicsWorld,mass};

        boost::apply_visitor(visitor,v);

        std::cout << ".. did it work?\n";

        gtl::rate_limiter rate_limiter_{std::chrono::milliseconds(15)};                                        

        while (quit_.test_and_set(std::memory_order_acquire))
        {
            rate_limiter_.sleepy_invoke([&](auto dt){
      
                local_debug_render_data_.clear();
                local_render_data_.clear();
                
                dynamicsWorld.stepSimulation(std::chrono::duration<float>(dt).count(), 4);                

                //http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Stepping_The_World

                b2d_adapter_.render(local_debug_render_data_);                

                for (auto&& e : entities_) { e.render(); }

                rend_data_.swap_in(local_render_data_);
            });
        }
        
        for (auto&& b : bodies_) {
            dynamicsWorld.removeRigidBody(&b);
        }


        dynamicsWorld.removeRigidBody(&fallRigidBody);
        dynamicsWorld.removeRigidBody(&groundRigidBody);        
    }
}
} // namespace