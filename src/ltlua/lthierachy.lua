lt.classes = {
    Object = {
        methods = {
            Tween                       = lt.Tween,
        }
    },

    -- Graphics
    SceneNode = {
        super = "Object",
        methods = {
            Draw                        = lt.DrawSceneNode,
            OnPointerUp                 = lt.AddOnPointerUpHandler,
            OnPointerDown               = lt.AddOnPointerDownHandler,
            OnPointerMove               = lt.AddOnPointerMoveHandler,
            OnPointerOver               = lt.AddOnPointerOverHandler,
            PropogatePointerUpEvent     = lt.PropogatePointerUpEvent,
            PropogatePointerDownEvent   = lt.PropogatePointerDownEvent,
            PropogatePointerMoveEvent   = lt.PropogatePointerMoveEvent,
            Tint                        = lt.Tint,
            BlendMode                   = lt.BlendMode,
            Translate                   = lt.Translate,
            Rotate                      = lt.Rotate,
            Scale                       = lt.Scale,
            Perspective                 = lt.Perspective,
            Pitch                       = lt.Pitch,
            HitFilter                   = lt.HitFilter,
            Wrap                        = lt.Wrap,
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
    BlendMode = {
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
    -- 3D
    Cuboid = {
        super = "SceneNode",
        methods = {}
    },
    Perspective = {
        super = "SceneNode",
        methods = {}
    },
    Pitch = {
        super = "SceneNode",
        methods = {}
    },
    HitFilter = {
        super = "SceneNode",
        methods = {}
    },
    Wrap = {
        super = "SceneNode",
        methods = {
            Replace         = lt.ReplaceWrappedChild,
        }
    },

    -- Audio
    Sample = {
        super = "Object",
        methods = {
            Play            = lt.PlaySampleOnce,
        }
    },
    Track = {
        super = "Object",
        methods = {
            Play            = lt.PlayTrack,
            Queue           = lt.QueueSampleInTrack,
            SetLoop         = lt.SetTrackLoop,
        }
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
    local lt_get = lt.GetObjectField
    local lt_set = lt.SetObjectField
    local function get(obj, field)
        local value = rawget(obj, field)
        if value then
            return value
        end
        value = lt_get(obj, field)
        if value then
            return value
        end
        local child = rawget(obj, "child")
        if child then
            return get(child, field)
        else
            return nil
        end
    end
    local function set(obj, field, value)
        local owner = obj
        local raw_field = false
        local lt_field = false
        while true do
            if rawget(owner, field) then
                raw_field = true
                break
            end
            if lt_get(owner, field) then
                lt_field = true
                break
            end
            local child = rawget(owner, "child")
            if child then
                owner = child
            else
                break
            end
        end
        if raw_field then
            rawset(owner, field, value)
            return
        end
        if lt_field then
            lt_set(owner, field, value)
            return
        end
        rawset(obj, field, value)
    end
    lt.metatables[class] = {
        __index = function(x, f) return method_index[f] or get(x, f) end,
        __newindex = set,
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
