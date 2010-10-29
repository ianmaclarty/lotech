dofile("../src/ltlua/lt.lua")

lt.SetViewPort(-10, -10, 10, 10)
local tweens = lt.TweenSet()

local xs = {
    -5,
    -5,
    -5,
    -5,
    -5,
    -5,
    -5,
    -5,
    -5
}
lt.AddTween(tweens, xs, 1, 5, 3, lt.LinearEase)
lt.AddTween(tweens, xs, 2, 5, 3, lt.EaseIn)
lt.AddTween(tweens, xs, 3, 5, 3, lt.EaseOut)
lt.AddTween(tweens, xs, 4, 5, 3, lt.EaseInOut)
lt.AddTween(tweens, xs, 5, 5, 3, lt.BackInEase)
lt.AddTween(tweens, xs, 6, 5, 3, lt.BackOutEase)
lt.AddTween(tweens, xs, 7, 5, 3, lt.ElasticEase)
lt.AddTween(tweens, xs, 8, 5, 3, lt.BounceEase)
lt.AddTween(tweens, xs, 9, 5, 3, lt.CubicBezierEase(0, 1, 1, 0))

function lt.Advance()
    lt.AdvanceTweens(tweens, lt.secs_per_frame)
end

function rect(x, y)
    lt.DrawRect(x - 0.5, y - 0.5, x + 0.5, y + 0.5)
end

function lt.Render()
    lt.SetColor(0, 1, 0)
    lt.DrawRect(5.5, -10, 6, 10)
    lt.SetColor(0, 0, 1)
    lt.DrawRect(-6, -10, -5.5, 10)
    lt.SetColor(1, 0, 0)
    for i = 1, 9 do
        rect(xs[i], -(i * 1.8) + 9)
    end
end
