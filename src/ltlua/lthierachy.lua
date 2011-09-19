-- Copyright 2011 Ian MacLarty
lt.classes = {
    Object = {
        methods = {
            Tween                       = lt.Tween,
            CancelTween                 = lt.CancelTween,
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
            DownFilter                  = lt.DownFilter,
            HitBarrier                  = lt.HitBarrier,
            Wrap                        = lt.Wrap,
            TrackBody                   = lt.BodyTracker,
            Button                      = lt.Button,
        }
    },
    Layer = {
        super = "SceneNode",
        methods = {
            Insert          = lt.InsertLayerFront,
            InsertBack      = lt.InsertLayerBack,
            InsertAbove     = lt.InsertLayerAbove,
            InsertBelow     = lt.InsertLayerBelow,
            Remove          = lt.RemoveFromLayer,
            Size            = lt.LayerSize,
        }
    },
    Translate = {
        super = "Wrap",
        methods = {}
    },
    Rotate = {
        super = "Wrap",
        methods = {}
    },
    Scale = {
        super = "Wrap",
        methods = {}
    },
    Tint = {
        super = "Wrap",
        methods = {}
    },
    BlendMode = {
        super = "Wrap",
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
    HitFilter = {
        super = "Wrap",
        methods = {}
    },
    DownFilter = {
        super = "Wrap",
        methods = {}
    },
    Wrap = {
        super = "SceneNode",
        methods = {
            Replace         = lt.ReplaceWrappedChild,
        }
    },

    -- 3D
    Cuboid = {
        super = "SceneNode",
        methods = {}
    },
    Perspective = {
        super = "Wrap",
        methods = {}
    },
    Pitch = {
        super = "Wrap",
        methods = {}
    },

    -- Vectors
    Vector = {
        super = "Object",
        methods = {
            GenerateColumn = lt.GenerateVectorColumn,
            FillWithImage  = lt.FillVectorColumnsWithImageQuads,
        },
    },
    DrawQuads = {
        super = "SceneNode",
        methods = {},
    },
    DrawVector = {
        super = "SceneNode",
        methods = {},
    },

    -- Particle System
    ParticleSystem = {
        super = "SceneNode",
        methods = {
            Advance = lt.ParticleSystemAdvance,
            FixtureFilter = lt.ParticleSystemFixtureFilter,
        }
    },

    -- Audio
    Sample = {
        super = "Object",
        methods = {
            Play            = lt.PlaySampleOnce,
            Length          = lt.SampleLength,
        }
    },
    Track = {
        super = "Object",
        methods = {
            Play            = lt.PlayTrack,
            Queue           = lt.QueueSampleInTrack,
            SetLoop         = lt.SetTrackLoop,
            NumQueued       = lt.TrackQueueSize,
            NumPending      = lt.TrackNumPending,
            NumPlayed       = lt.TrackNumPlayed,
            Dequeue         = lt.TrackDequeuePlayed,
        }
    },

    -- Physics
    World = {
        super = "Object",
        methods = {
            Step            = lt.DoWorldStep,
            SetGravity      = lt.SetWorldGravity,
            SetAutoClearForces = lt.SetWorldAutoClearForces,
            QueryBox        = lt.WorldQueryBox,
            AddStaticBody   = lt.AddStaticBodyToWorld,
            AddDynamicBody  = lt.AddDynamicBodyToWorld,
            AddBody         = lt.AddBodyToWorld,
            RayCast         = lt.WorldRayCast,
        }
    },
    Body = {
        super = "SceneNode",
        methods = {
            Destroy             = lt.DestroyBody,
            IsDestroyed         = lt.BodyIsDestroyed,
            ApplyForce          = lt.ApplyForceToBody,
            ApplyTorque         = lt.ApplyTorqueToBody,
            ApplyImpulse        = lt.ApplyImpulseToBody,
            ApplyAngularImpulse = lt.ApplyAngularImpulseToBody,
            GetAngle            = lt.GetBodyAngle,
            SetAngle            = lt.SetBodyAngle,
            GetPosition         = lt.GetBodyPosition,
            SetPosition         = lt.SetBodyPosition,
            GetVelocity         = lt.GetBodyVelocity,
            SetVelocity         = lt.SetBodyVelocity,
            SetAngularVelocity  = lt.SetBodyAngularVelocity,
            AddRect             = lt.AddRectToBody,
            AddTriangle         = lt.AddTriangleToBody,
            AddPoly             = lt.AddPolygonToBody,
            AddCircle           = lt.AddCircleToBody,
            Touching            = lt.BodyOrFixtureTouching,
        }
    },
    Fixture = {
        super = "SceneNode",
        methods = {
            ContainsPoint   = lt.FixtureContainsPoint,
            Destroy         = lt.DestroyFixture,
            IsDestroyed     = lt.FixtureIsDestroyed,
            GetBody         = lt.GetFixtureBody,
            Touching        = lt.BodyOrFixtureTouching,
        }
    },
    BodyTracker = {
        super = "Wrap",
        methods = {}
    },
}

-- Populate lt.metatables.
lt.metatables = {}
local lt_get = lt.GetObjectField
local lt_set = lt.SetObjectField
local function get(obj, field, is_child)
    local value = rawget(obj, field)
    if value then
        return value
    end
    value = lt_get(obj, field)
    if value then
        return value
    end
    local mt = getmetatable(obj)
    value = rawget(mt, field)
    if value then
        if is_child and type(value) == "function" then
            -- Dynamiclly generate a method that will have the child
            -- node as its self value.
            return function(_, ...)
                return value(obj, ...)
            end
        else
            return value
        end
    end
    local child = rawget(obj, "child")
    if child then
        return get(child, field, true)
    else
        return nil
    end
end
local function set(obj, field, value)
    local owner = obj
    while true do
        if rawget(owner, field) then
            rawset(owner, field, value)
            return
        end
        if lt_get(owner, field) then
            lt_set(owner, field, value)
            return
        end
        local child = rawget(owner, "child")
        if child then
            owner = child
        else
            rawset(obj, field, value)
            return
        end
    end
end
for class, info in pairs(lt.classes) do
    local metatable = {
        __index = get,
        __newindex = set,
    }
    lt.metatables[class] = metatable
    metatable.class = class
    while info do
        for method, impl in pairs(info.methods) do
            if not metatable[method] then 
                metatable[method] = impl
            end
        end
        info = lt.classes[info.super]
    end
end
