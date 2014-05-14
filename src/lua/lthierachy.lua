-- Copyright (C) 2010-2013 Ian MacLarty. See Copyright Notice in ../lt.h
local descendents = {}
for name, mt in pairs(lt_metatables) do
    for ancestor, _ in pairs(mt.is) do
        local ds = descendents[ancestor]
        if not ds then
            ds = {}
            descendents[ancestor] = ds
        end
        ds[name] = mt
    end
end

local
function mt_add(name, field, val)
    for desc, mt in pairs(descendents[name]) do
        mt[field] = val
    end
end

mt_add("lt.SceneNode", "Tween", lt.Tween)
mt_add("lt.SceneNode", "CancelTween", lt.CancelTween)
mt_add("lt.SceneNode", "Draw", lt.DrawSceneNode)
mt_add("lt.SceneNode", "Tint", lt.Tint)
mt_add("lt.SceneNode", "ColorMask", lt.ColorMask)
mt_add("lt.SceneNode", "BlendMode", lt.BlendMode)
mt_add("lt.SceneNode", "TextureMode", lt.TextureMode)
mt_add("lt.SceneNode", "Translate", lt.Translate)
mt_add("lt.SceneNode", "Rotate", lt.Rotate)
mt_add("lt.SceneNode", "Scale", lt.Scale)
mt_add("lt.SceneNode", "Shear", lt.Shear)
mt_add("lt.SceneNode", "Transform", lt.Transform)
mt_add("lt.SceneNode", "Perspective", lt.Perspective)
mt_add("lt.SceneNode", "Pitch", lt.Pitch)
mt_add("lt.SceneNode", "HitFilter", lt.HitFilter)
mt_add("lt.SceneNode", "DownFilter", lt.DownFilter)
mt_add("lt.SceneNode", "HitBarrier", lt.HitBarrier)
mt_add("lt.SceneNode", "Wrap", lt.Wrap)
mt_add("lt.SceneNode", "TrackBody", lt.BodyTracker)
mt_add("lt.SceneNode", "Button", lt.Button)
mt_add("lt.SceneNode", "Fog", lt.Fog)
mt_add("lt.SceneNode", "DepthTest", lt.DepthTest)
mt_add("lt.SceneNode", "DepthMask", lt.DepthMask)
mt_add("lt.SceneNode", "ContainsPoint", lt.SceneNodeContainsPoint)
mt_add("lt.SceneNode", "Action", lt.AddAction)
mt_add("lt.SceneNode", "Lighting", lt.Lighting)
mt_add("lt.SceneNode", "Light", lt.Light)
mt_add("lt.SceneNode", "Material", lt.Material)
mt_add("lt.SceneNode", "CullFace", lt.CullFace)
mt_add("lt.SceneNode", "Hidden", lt.Hidden)
mt_add("lt.SceneNode", "RenderTarget", lt.RenderTarget)

mt_add("lt.SceneNode", "Event", lt.AddEventHandler)
mt_add("lt.SceneNode", "Mouse", lt.AddMouseHandler)
mt_add("lt.SceneNode", "MouseDown", lt.AddMouseDownHandler)
mt_add("lt.SceneNode", "MouseUp", lt.AddMouseUpHandler)
mt_add("lt.SceneNode", "MouseMove", lt.AddMouseMoveHandler)
mt_add("lt.SceneNode", "MouseEnter", lt.AddMouseEnterHandler)
mt_add("lt.SceneNode", "MouseExit", lt.AddMouseExitHandler)
mt_add("lt.SceneNode", "Touch", lt.AddTouchHandler)
mt_add("lt.SceneNode", "TouchDown", lt.AddTouchDownHandler)
mt_add("lt.SceneNode", "TouchUp", lt.AddTouchUpHandler)
mt_add("lt.SceneNode", "TouchMove", lt.AddTouchMoveHandler)
mt_add("lt.SceneNode", "TouchEnter", lt.AddTouchEnterHandler)
mt_add("lt.SceneNode", "TouchExit", lt.AddTouchExitHandler)
mt_add("lt.SceneNode", "Pointer", lt.AddPointerHandler)
mt_add("lt.SceneNode", "PointerDown", lt.AddPointerDownHandler)
mt_add("lt.SceneNode", "PointerUp", lt.AddPointerUpHandler)
mt_add("lt.SceneNode", "PointerMove", lt.AddPointerMoveHandler)
mt_add("lt.SceneNode", "PointerEnter", lt.AddPointerEnterHandler)
mt_add("lt.SceneNode", "PointerExit", lt.AddPointerExitHandler)
mt_add("lt.SceneNode", "Key", lt.AddKeyHandler)
mt_add("lt.SceneNode", "KeyDown", lt.AddKeyDownHandler)
mt_add("lt.SceneNode", "KeyUp", lt.AddKeyUpHandler)
mt_add("lt.SceneNode", "PropagateEvent", lt.PropagateEvent)
mt_add("lt.SceneNode", "Exclusive", lt.MakeSceneNodeExclusive)

mt_add("lt.Layer", "Insert", lt.InsertLayerFront)
mt_add("lt.Layer", "InsertBack", lt.InsertLayerBack)
mt_add("lt.Layer", "InsertAbove", lt.InsertLayerAbove)
mt_add("lt.Layer", "InsertBelow", lt.InsertLayerBelow)
mt_add("lt.Layer", "Remove", lt.RemoveFromLayer)
mt_add("lt.Layer", "Size", lt.LayerSize)

mt_add("lt.VectorImpl", "GenerateColumn", lt.GenerateVectorColumn)
mt_add("lt.VectorImpl", "FillWithImage", lt.FillVectorColumnsWithImageQuads)

--lt.TweenSet_mt.Advance = lt.AdvanceTweens
--lt.TweenSet_mt.Add = lt.AddTweens
--lt.TweenSet_mt.Clear = lt.ClearTweens

mt_add("lt.Sample", "Play", lt.PlaySampleOnce)
mt_add("lt.Sample", "Length", lt.SampleLength)

mt_add("lt.Track", "Play", lt.PlayTrack)
mt_add("lt.Track", "Pause", lt.PauseTrack)
mt_add("lt.Track", "Stop", lt.StopTrack)
mt_add("lt.Track", "Rewind", lt.RewindTrack)
mt_add("lt.Track", "Queue", lt.QueueSampleInTrack)
mt_add("lt.Track", "SetLoop", lt.SetTrackLoop)
mt_add("lt.Track", "NumQueued", lt.TrackQueueSize)
mt_add("lt.Track", "NumPending", lt.TrackNumPending)
mt_add("lt.Track", "NumPlayed", lt.TrackNumPlayed)
mt_add("lt.Track", "Dequeue", lt.TrackDequeuePlayed)

--[[
lt.World_mt.Step            = lt.DoWorldStep
lt.World_mt.SetGravity      = lt.SetWorldGravity
lt.World_mt.SetAutoClearForces = lt.SetWorldAutoClearForces
lt.World_mt.QueryBox        = lt.WorldQueryBox
lt.World_mt.AddStaticBody   = lt.AddStaticBodyToWorld
lt.World_mt.AddDynamicBody  = lt.AddDynamicBodyToWorld
lt.World_mt.AddBody         = lt.AddBodyToWorld
lt.World_mt.RayCast         = lt.WorldRayCast
lt.World_mt.AddJoint        = lt.AddJointToWorld

lt.Body_mt.Destroy             = lt.DestroyBody
lt.Body_mt.IsDestroyed         = lt.BodyIsDestroyed
lt.Body_mt.ApplyForce          = lt.ApplyForceToBody
lt.Body_mt.ApplyTorque         = lt.ApplyTorqueToBody
lt.Body_mt.ApplyImpulse        = lt.ApplyImpulseToBody
lt.Body_mt.ApplyAngularImpulse = lt.ApplyAngularImpulseToBody
lt.Body_mt.GetAngle            = lt.GetBodyAngle
lt.Body_mt.SetAngle            = lt.SetBodyAngle
lt.Body_mt.GetPosition         = lt.GetBodyPosition
lt.Body_mt.SetPosition         = lt.SetBodyPosition
lt.Body_mt.GetVelocity         = lt.GetBodyVelocity
lt.Body_mt.SetVelocity         = lt.SetBodyVelocity
lt.Body_mt.SetAngularVelocity  = lt.SetBodyAngularVelocity
lt.Body_mt.SetGravityScale     = lt.SetBodyGravityScale
lt.Body_mt.AddRect             = lt.AddRectToBody
lt.Body_mt.AddTriangle         = lt.AddTriangleToBody
lt.Body_mt.AddPoly             = lt.AddPolygonToBody
lt.Body_mt.AddCircle           = lt.AddCircleToBody
lt.Body_mt.Touching            = lt.BodyOrFixtureTouching
lt.Body_mt.Fixtures            = lt.GetBodyFixtures

lt.Fixture_mt.ContainsPoint   = lt.FixtureContainsPoint
lt.Fixture_mt.Destroy         = lt.DestroyFixture
lt.Fixture_mt.IsDestroyed     = lt.FixtureIsDestroyed
lt.Fixture_mt.GetBody         = lt.GetFixtureBody
lt.Fixture_mt.Touching        = lt.BodyOrFixtureTouching
lt.Fixture_mt.BoundingBox     = lt.FixtureBoundingBox
]]

mt_add("lt.Random", "Int", lt.NextRandomInt)
mt_add("lt.Random", "Num", lt.NextRandomNumber)
mt_add("lt.Random", "Bool", lt.NextRandomBool)
