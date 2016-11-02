/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

#include "gtl/physics/generators/simple_boundary.h"

//#include <gtl/physics/simulation_interface.h>
#include <gtl/physics/common_types.h>

#include <Box2D/Box2D.h>
#include <gtl/physics/common_categories.h>

namespace gtl {
namespace physics {
    namespace generators {

        static void add_static_rect(b2World& world, float x, float y, float w, float h)
        {
            b2BodyDef body{};

            body.position.x = x;
            body.position.y = y;

            body.type = b2_staticBody;

            auto* body_ptr = world.CreateBody(&body);

            b2PolygonShape shape;
            shape.SetAsBox(w * 0.5f, h * 0.5f);

            b2FixtureDef fixture{};
            fixture.isSensor = false;
            fixture.restitution = 0.5f;
            fixture.density = 1.0f; // shouldn't matter with static body
            fixture.friction = 0.1f;

            fixture.filter.categoryBits = collision_category::BOUNDARY;
            fixture.filter.maskBits = collision_category::ENTITY;
            fixture.shape = &shape;

            body_ptr->CreateFixture(&fixture);
        }

        void simple_boundary::apply(b2World& world) const
        {
            using namespace boost::units;

            auto w = dimensions_.first / si::meters;
            auto h = dimensions_.second / si::meters;
            auto x = center_.first / si::meters;
            auto y = center_.second / si::meters;

            add_static_rect(world, x - 0.5f * w, y, 2.0f, h + 2.0f); // left wall
            add_static_rect(world, x + 0.5f * w, y, 2.0f, h + 2.0f); // right wall

            add_static_rect(world, x, y + 0.5f * h, w + 2.0f, 2.0f); // top wall
            add_static_rect(world, x, y - 0.5f * h, w + 2.0f, 2.0f); // bottom wall
        }
    }
}
} // namespace
