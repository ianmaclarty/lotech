lt.classes = {
    Object = {
        methods = {
            Set = lt.SetObjectFields,
        }
    },

    -- Graphics
    SceneNode = {
        super = "Object",
        methods = {
            Draw                        = lt.DrawSceneNode,
            OnPointerDown               = lt.AddOnPointerDownHandler,
            OnPointerOver               = lt.AddOnPointerOverHandler,
            PropogatePointerDownEvent   = lt.PropogatePointerDownEvent,
            PropogatePointerMoveEvent   = lt.PropogatePointerMoveEvent,
        }
    },
    Layer = {
        super = "SceneNode",
        methods = {
            Insert          = lt.InsertIntoLayer,
            Remove          = lt.RemoveFromLayer,
        }
    },
    Translate = {
        super = "SceneNode",
        methods = {}
    },
    Rotate = {
        super = "SceneNode",
        methods = {}
    },
    Scale = {
        super = "SceneNode",
        methods = {}
    },
    Tint = {
        super = "SceneNode",
        methods = {}
    },
    Line = {
        super = "SceneNode",
        methods = {}
    },
    Triangle = {
        super = "SceneNode",
        methods = {}
    },
    Rect = {
        super = "SceneNode",
        methods = {}
    },
    Image = {
        super = "SceneNode",
        methods = {}
    },

    -- Physics
    World = {
        super = "Object",
        methods = {
            Step            = lt.DoWorldStep,
            SetGravity      = lt.SetWorldGravity,
            QueryBox        = lt.WorldQueryBox,
            AddStaticBody   = lt.AddStaticBodyToWorld,
            AddDynamicBody  = lt.AddDynamicBodyToWorld,
        }
    },
    Body = {
        super = "SceneNode",
        methods = {
            Destroy     = lt.DestroyBody,
            IsDestroyed = lt.BodyIsDestroyed,
            ApplyForce  = lt.ApplyForceToBody,
            ApplyTorque = lt.ApplyTorqueToBody,
            GetAngle    = lt.GetBodyAngle,
            SetAngle    = lt.SetBodyAngle,
            GetPosition = lt.GetBodyPosition,
            SetAngularVelocity = lt.SetBodyAngularVelocity,
            AddRect     = lt.AddRectToBody,
            AddTriangle = lt.AddTriangleToBody,
        }
    },
    Fixture = {
        super = "SceneNode",
        methods = {
            ContainsPoint   = lt.FixtureContainsPoint,
            Destroy         = lt.DestroyFixture,
            IsDestroyed     = lt.FixtureIsDestroyed,
            GetBody         = lt.GetFixtureBody,
        }
    },
}

-- Populate lt.metatables.
lt.metatables = {}
for class, info in pairs(lt.classes) do
    local method_index = {}
    local get = lt.GetObjectField
    lt.metatables[class] = {
        __index = function(x, f) return method_index[f] or get(x, f) end,
        __newindex = lt.SetObjectField,
    }
    method_index.class = class
    while info do
        for method, impl in pairs(info.methods) do
            if not method_index[method] then 
                method_index[method] = impl
            end
        end
        info = lt.classes[info.super]
    end
end
