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
#include <utility>
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <exception>
#include <unordered_map>


#include <vn/single_consumer_queue.h>
#include <vn/swap_object.h>

#include <gtl/physics/common_types.h>
#include <gtl/physics/simulation_render_data.h>
#include <gtl/physics/command_variant.h>
#include <gtl/draw_kit.h>               // HACK change this..

#include <gtl/entity/entity_id.h>
#include <gtl/entity/render_data.h>

#include <gtl/rate_limiter.h>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <vn/math_utilities.h>
#include <vn/boost_variant_utilities.h>

#include <boost/container/flat_map.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/unordered_map.hpp>

// Bullet physics

#pragma warning(disable : 4305)
#include <btBulletDynamicsCommon.h>

// Notes on stepping: http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Stepping_The_World
//
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
                                         gtl::draw_kit& b2d_adapter_);    
}

namespace physics {
    bullet_simulation::bullet_simulation(vn::single_consumer_queue<gtl::physics::command_variant>& tasks_, 
                                         gtl::draw_kit& b2d_adapter_)
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
            
    enum class shape_enum {
        BOX,
        SPHERE,
        OTHER
    };

    template <typename W, typename M, typename S>
    static auto make_command_visitor(W& world, M& entities, S& shapes) {
        namespace commands = gtl::physics::commands;
        return vn::make_lambda_visitor(
        [&](commands::static_box const& o)
           { 
               return true;  
           }
        ,[&](commands::dynamic_box const& o)       
           {   
               using namespace boost::units;           

               std::unique_ptr<btCollisionShape> my_shape = 
                   std::make_unique<btBoxShape>(btVector3{o.half_extents_.x(),o.half_extents_.y(),o.half_extents_.z()});               

               entities.emplace(boost::unordered::piecewise_construct,
                                std::forward_as_tuple(o.id_),
                                std::forward_as_tuple(world,
                                                      o.render_data_,
                                                      o.mass_ / si::kilograms,
                                                      my_shape.get(),
                                                      btTransform{ btQuaternion{o.orientation_.x(),
                                                                                o.orientation_.y(),
                                                                                o.orientation_.z(),
                                                                                o.orientation_.w()},
                                                                    btVector3{o.xyz_.x(),o.xyz_.y(),o.xyz_.z()} } 
                                                                   ));
               entities.at(o.id_).visit(
                   [](auto& b){
                       b.setRestitution(0.4f);
                       b.setDamping(0.1f,0.1f);
                       b.activate();
                   });  

               shapes.emplace(shape_enum::BOX, std::move(my_shape));
               return true;
           }                                                
        ,[&](commands::dynamic_jointed_boxes const& o)       
           {   
               using namespace boost::units;           

               std::unique_ptr<btCollisionShape> my_shape = 
                   std::make_unique<btBoxShape>(btVector3{1.0f,1.0f,1.0f});               

               std::vector<std::tuple<btScalar, btCollisionShape*, btTransform>> local_bodies_;

               for (auto&& e : o.boxes_) {
                   local_bodies_.emplace_back(e.mass_ / si::kilograms, my_shape.get(), 
                                              btTransform{ btQuaternion{e.orientation_.x(),
                                                                        e.orientation_.y(),
                                                                        e.orientation_.z(),
                                                                        e.orientation_.w()},
                                                           btVector3{e.xyz_.x(),e.xyz_.y(),e.xyz_.z()}
                                                          });                                                       
               }

               entities.emplace(boost::unordered::piecewise_construct,
                                std::forward_as_tuple(o.id_),
                                std::forward_as_tuple(world,
                                                      o.render_data_,
                                                      std::move(local_bodies_)));
               
               entities.at(o.id_).visit(
                   [](auto& b){
                       b.setRestitution(0.4f);
                       b.setDamping(0.1f,0.1f);
                       b.activate();
                   });  

               shapes.emplace(shape_enum::BOX, std::move(my_shape));
               return true;
           }                                                
        ,[&](commands::destroy_object_implode const& o)
           { 
               if (entities.count(o.id_) > 0)
                    entities.erase(o.id_);
               return true;  
           }
        ,[&](commands::boost_object const& o)
           {                
               std::cout << "boosting..\n"; 
               if (entities.count(o.id_) > 0) {
                   btRigidBody& b = entities.at(o.id_).head().get_body();
                   b.activate();
                   b.applyImpulse(btVector3{ 0.0f,1000.0f,0.0f }, btVector3{ 0.0f,0.0f,0.0f });
               }
               return true;  
           }
        ,[&](commands::boost_object_vec const& o)
           { 
               return true;  
           }
        ,[&](commands::drive_object_vec const& o)
           { 
               return true;  
           }       
        ,[](auto const&) { assert(false); return false; } // shouldn't be called..
        
        );
    }
    
//###########################################################

    class bullet_body : public btMotionState {                
        btTransform transform_;                               
        btRigidBody body_;        
    
    public:        
        bullet_body(btScalar mass, btCollisionShape* shape_ptr, btTransform const& init_trans, btVector3 inertia = btVector3{0.0f,0.0f,0.0f})               
            :   transform_(init_trans),
            body_(btRigidBody::btRigidBodyConstructionInfo{mass, this, shape_ptr, [&]{ shape_ptr->calculateLocalInertia(mass, inertia); return inertia; }()})                
        {}          
        
        bullet_body(bullet_body&&) = delete;        

        ~bullet_body() final {}

        btRigidBody& get_body() { return body_; }
        btTransform const& get_transform() const { return transform_; }

        virtual void getWorldTransform(btTransform& t) const final { t = transform_; }
        virtual void setWorldTransform(btTransform const& t) final { transform_ = t; }                       
    };
    
    class bullet_entity {
        boost::container::static_vector<bullet_body, 4> participating_bodies_;
        boost::container::static_vector<btConeTwistConstraint, 3> constraints_;
        entity::render_data render_data_;
        btDynamicsWorld& world_;
    
        void    load() { for (auto&& e : participating_bodies_) { world_.addRigidBody(&e.get_body()); } 
                         for (auto&& e : constraints_) { world_.addConstraint(&e,true); } }
        void  unload() { for (auto&& e : constraints_) { world_.removeConstraint(&e); }
                         for (auto&& e : participating_bodies_) { world_.removeRigidBody(&e.get_body()); } }        

    public:

        bullet_entity(btDynamicsWorld& world, entity::render_data r, std::vector<std::tuple<btScalar, btCollisionShape*, btTransform>> ci) 
            : render_data_{r}, 
              world_{world}
        {                        
            for (auto&& e : ci) { 
                participating_bodies_.emplace_back(std::get<0>(e),std::get<1>(e),std::get<2>(e)); // throws if it goes beyond capacity                
            }

            for (int i = 0, sz = static_cast<int>(participating_bodies_.size()); i < sz-1; ++i) {                
                btConeTwistConstraint constraint_{participating_bodies_[i].get_body(), 
                                                  participating_bodies_[i+1].get_body(), 
                                                  btTransform{btQuaternion::getIdentity(),btVector3{0.0f,0.0f,-1.0f}}, 
                                                  btTransform{btQuaternion::getIdentity(),btVector3{0.0f,0.0f,1.0f}}};
                
                constraint_.setLimit(vn::math::pi() * 0.15f, vn::math::pi() * 0.15f, 0.0f);

                constraints_.emplace_back(constraint_);
            }
            std::cout << "multiple object " << r.entity_id() << "," << participating_bodies_.size() << "," << constraints_.size() << "\n"; 
            load();
        }

        bullet_entity(btDynamicsWorld& world, entity::render_data r, btScalar mass, btCollisionShape* shape, btTransform transform) 
            : render_data_{r}, 
              world_{world}
        {            
            participating_bodies_.emplace_back(mass, shape, transform);
            std::cout << "single object " << r.entity_id() << "," << participating_bodies_.size() << "," << constraints_.size() << "\n"; "\n"; 
            load();
        }

        bullet_entity(bullet_entity const&) = delete;    
    
        ~bullet_entity() { unload(); }

        template <typename F>
        void visit(F func) { for (auto& e : participating_bodies_) { func(e.get_body()); } };
        bullet_body& head() { return participating_bodies_[0]; }
        
        void render(std::vector<entity::render_data>& render_queue, std::vector<Eigen::Matrix4f>& transform_queue) {        
            render_queue.emplace_back(render_data_.pack_bone_offset(static_cast<uint16_t>(transform_queue.size())));                                    
            Eigen::Matrix4f tmp = Eigen::Matrix4f::Identity();
                        
//            auto& h = participating_bodies_[0].get_body();
            
            for (auto&& e : participating_bodies_) {                                              
              //e.get_body().getWorldTransform().getOpenGLMatrix(tmp.data());
              e.get_transform().getOpenGLMatrix(tmp.data());             
              //h.getWorldTransform().getOpenGLMatrix(tmp.data());
              transform_queue.emplace_back(tmp.transpose());                    
            }              
        }

    };



//###########################################################
//###########################################################

    template <typename F> 
    struct debug_draw : btIDebugDraw {
        static constexpr int max_lines = 2000;
        
        F func_;
        int debug_mode_;
        int line_count_{};
     
        constexpr debug_draw(int mode, F&& f) noexcept : func_{std::move(f)}, debug_mode_{mode} {}
        virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color) final {
            if (++line_count_ < max_lines)
                func_(from,to,color);
        }        
        virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color) final {}
        virtual void reportErrorWarning(const char* warningString) final {}
        virtual void draw3dText(const btVector3& location,const char* textString) final {}	
        virtual void setDebugMode(int debugMode) final { debug_mode_ = debugMode; }
        virtual int  getDebugMode() const final { return debug_mode_; }

        void clear() { line_count_ = 0; }

        ~debug_draw() final {}
    };

    template <typename F>
    auto make_debug_draw(int debug_mode_flags, F func) { return debug_draw<F>{debug_mode_flags,std::move(func)}; }

 //############################################################################################

    template <typename T, typename U>
    static void bullet_simulation_thread(
        vn::swap_object<T>& rend_data_, 
        vn::single_consumer_queue<U>& physics_task_queue_,        
        std::atomic_flag& quit_, 
        gtl::draw_kit& b2d_adapter_)
    {        
        btDbvtBroadphase broadphase{};
        btDefaultCollisionConfiguration collisionConfiguration{};
        btCollisionDispatcher dispatcher{&collisionConfiguration};
        btSequentialImpulseConstraintSolver solver{};
        btDiscreteDynamicsWorld dynamicsWorld{&dispatcher, &broadphase, &solver, &collisionConfiguration};            

        dynamicsWorld.setGravity(btVector3{0.0f, -8.0f, 0.0f});

        btStaticPlaneShape groundShape{btVector3(0.0f, 1.0f, 0.0f), 1.0f};        
        //btSphereShape fallShape{1.0f};
        btBoxShape fallShape{btVector3{1.0f,1.0f,1.0f}};

        btDefaultMotionState groundMotionState{btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f), btVector3(0.0f, -1.0f, 0.0f))};
        btRigidBody::btRigidBodyConstructionInfo
            groundRigidBodyCI{0, &groundMotionState, &groundShape};
        
        btRigidBody groundRigidBody{groundRigidBodyCI};
        
        dynamicsWorld.addRigidBody(&groundRigidBody);        

        // dynamicsWorld.setInternalTickCallback()        
        
        boost::container::flat_multimap<shape_enum, std::unique_ptr<btCollisionShape>> shapes_;
        boost::unordered_map<entity::id, bullet_entity> entities_;
                   
        auto my_command_visitor = make_command_visitor(dynamicsWorld, entities_, shapes_);
        physics_task_queue_.consume([&](auto&& e){ my_command_visitor(e); });
      
        gtl::physics::simulation_render_data local_render_data_;                        

        gtl::draw_data local_debug_render_data_; 
        using dd = btIDebugDraw::DebugDrawModes;
        auto debug_draw = make_debug_draw( dd::DBG_DrawWireframe | dd::DBG_NoHelpText | dd::DBG_FastWireframe,
            [&](auto const& a, auto const& b, auto const& color) { 
                local_debug_render_data_.draw_line({a.x(),a.y(),a.z()},{b.x(),b.y(),b.z()},{color.x(),color.y(),color.z()});
            });         
        
        //dynamicsWorld.setDebugDrawer(&debug_draw);                      

        gtl::rate_limiter rate_limiter_{std::chrono::milliseconds(15)};                     

        int counter{};

        while (quit_.test_and_set(std::memory_order_acq_rel))
        {
            rate_limiter_.sleepy_invoke([&](auto dt){
      
                local_debug_render_data_.clear();
                local_render_data_.clear();
  
                debug_draw.clear();
                
                physics_task_queue_.consume([&](auto&& e){ my_command_visitor(e); });                
                
                counter++;
                if (counter > 100) {
                    std::cout << std::chrono::duration<float>(dt).count() << " seconds for dt..\n";
                    counter = 0;
                }

                dynamicsWorld.stepSimulation(std::chrono::duration<float>(dt).count(), 5);                                
                local_debug_render_data_.draw_axes();
                local_debug_render_data_.draw_triangle({0.0f,0.0f,0.0f,1.0f},{0.0f,1.0f,0.0f,1.0f},{1.0f,0.0f,0.0f,1.0f});                
          
                //dynamicsWorld.debugDrawWorld();

                b2d_adapter_.render(local_debug_render_data_);                

                for (auto&& e : entities_) { 
                    e.second.render(local_render_data_.entities_, local_render_data_.control_points_); 
                }

                rend_data_.swap_in(local_render_data_);

            });
        }
        
        dynamicsWorld.removeRigidBody(&groundRigidBody);        
    }
}
} // namespace


/*
    //template <typename Get, typename Set>
    //struct motion_state_helper: btMotionState, Get, Set {
    //    
    //    btTransform default_transform_;
    //            
    //    template <typename G, typename S, typename ...Ts>
    //    motion_state_helper(G&& get, S&& set, Ts&&...ts) 
    //        : Get(std::forward<G>(get)), 
    //          Set(std::forward<S>(set)),
    //          default_transform_(std::forward<Ts>(ts)...)
    //    {}
    //    
    //    virtual void getWorldTransform(btTransform &worldTrans) const final {
    //        (Get::operator())(worldTrans, default_transform_);
    //    }
    //
    //    virtual void setWorldTransform(btTransform const& worldTrans) final {
    //        (Set::operator())(worldTrans);
    //    }
    //    
    //    ~motion_state_helper() final {}
    //};
    //
    //template <typename Get, typename Set, typename ...Ts>
    //auto make_motion_state_get_set(Get&& get, Set&& set, Ts&&...ts)
    //{
    //    return motion_state_helper<std::remove_reference_t<Get>,
    //                               std::remove_reference_t<Set>>
    //                                    {
    //                                        std::forward<Get>(get),
    //                                        std::forward<Set>(set), 
    //                                        std::forward<Ts>(ts)...
    //                                    };
    //}     
    //
    //class BulletEntity : public btMotionState {        
    //    btTransform def_transform_;
    //    btScalar mass_;
    //    btVector3 fall_inertia_;
    //    btBoxShape shape_;                
    //            
    //    Eigen::Matrix4f my_transform_;
    //    gtl::entity::render_data entity_data_;               
    //
    //public:        
    //    BulletEntity(btTransform transform, 
    //                 btScalar mass, 
    //                 btVector3 fall_inertia,
    //                 btScalar shaperadius,
    //                 gtl::entity::render_data render_data,
    //                 gtl::physics::simulation_render_data& rdata) 
    //        : def_transform_{transform},
    //          mass_{mass},
    //          fall_inertia_{fall_inertia},
    //          shape_{btVector3{shaperadius,shaperadius,shaperadius}},       
    //          my_transform_{},
    //          entity_data_{render_data}              
    //    {
    //        setWorldTransform(transform);
    //    }
    //
    //    btRigidBody::btRigidBodyConstructionInfo rigidBodyInfo() {
    //        shape_.calculateLocalInertia(mass_,fall_inertia_);
    //        return {mass_,this,&shape_,fall_inertia_};
    //    }
    //
    //    ~BulletEntity() final {}
    //
    //    virtual void getWorldTransform(btTransform &worldTrans) const final {
    //        worldTrans = def_transform_;
    //    }
    //
    //    virtual void setWorldTransform(btTransform const& worldTrans) final {        
    //        float raw_[16];           
    //        worldTrans.getOpenGLMatrix(raw_);                        
    //        my_transform_ = Eigen::Map<Eigen::Matrix<float,4,4,Eigen::RowMajor>>{raw_};            
    //    }               
    //
    //    void render(gtl::physics::simulation_render_data& render_data_) {        
    //        render_data_.entities_.emplace_back(entity_data_);
    //        render_data_.control_points_.emplace_back(my_transform_);
    //    }
    //};
*/