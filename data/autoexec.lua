bind("backspace", [[if value~=0 then clear() end]])
bind("return", [[if value~=0 then load("world.txt") end]])
bind("1", [[if value~=0 then spawn("box") end]])
bind("2", [[if value~=0 then spawn("car") end]])
bind("3", [[if value~=0 then spawn("house") end]])
bind("4", [[if value~=0 then spawn("knife") end]])
bind("5", [[if value~=0 then spawn("room") end]])
bind("6", [[if value~=0 then spawn("sound") end]])
bind("7", [[if value~=0 then spawn("spin") end]])
bind("8", [[if value~=0 then spawn("spin2") end]])
bind("9", [[if value~=0 then spawn("trampoline") end]])
bind("0", [[if value~=0 then spawn("trampoline2") end]])
bind("w", "w=value")
bind("s", "s=value")
bind("a", "a=value")
bind("d", "d=value")
bind("f", "f=value")
bind("v", "v=value")
bind("up",   "key_up=value")
bind("down", "key_down=value")
bind("left", "key_left=value")
bind("right","key_right=value")
bind("frame", "frame(value)")
bind("joy0_axis0", "axis0=value")
bind("joy0_axis1", "axis1=value")
bind("joy0_axis2", "axis2=value")
bind("joy0_axis3", "axis3=value")
bind("joy0_axis4", "axis4=value")
bind("joy0_axis5", "axis5=value")
bind("joy0_axis6", "axis6=value")
bind("joy0_axis7", "axis7=value")
bind("joy0_axis8", "axis8=value")
bind("joy0_axis9", "axis9=value")
bind("joy0_axis10", "axis10=value")
bind("joy0_axis11", "axis11=value")
bind("joy0_axis12", "axis12=value")
bind("joy0_axis13", "axis13=value")
bind("mouse_relx", [[player("mousex " .. value)]])
bind("mouse_rely", [[player("mousey " .. value)]])

w, s, a, d, f, v = 0, 0, 0, 0, 0, 0
forward, back, left, right = 0, 0, 0, 0
key_up, key_down, key_left, key_right = 0, 0, 0, 0

axis0=0
axis1=0
axis2=-1
axis3=0
axis4=0
axis5=-1
axis6=0
axis7=0
axis8=0
axis9=0
axis10=0
axis11=0
axis12=0
axis13=0

dead_zone = 0.1

function max(a, b)
    return a > b and a or b
end

function lerp(a, b, t)
    return a + (b - a) * t
end

function remap_clamp(low_in, high_in, value_in, low_out, high_out)
    local t = (value_in - low_in) / (high_in - low_in)
    if t < 0 then
        return low_out
    end
    if t > 1 then
        return high_out
    end
    return low_out + (high_out - low_out) * t
end

function frame(value)
    forward = max(key_up, remap_clamp(-1, 1, axis5, 0, 1))
    back = max(key_down, remap_clamp(-1, 1, axis2, 0, 1))

    left = max(key_left, remap_clamp(dead_zone, 1, -axis0, 0, 1))
    right = max(key_right, remap_clamp(dead_zone, 1, axis0, 0, 1))

    player("velx " .. (w - s))
    player("vely " .. (a - d))
    player("velz " .. (f - v))
    player("motor Kaross_Axis_F 0 " .. (forward - back)*2000 .. " 0")
    player("motor Kaross_Axis_B 0 " .. (forward - back)*2000 .. " 0")
    player("motor engine1 0 " .. (forward - back)*200 .. " 0")
    player("motor engine2 0 " .. (forward - back)*200 .. " 0")
    player("motor engine 0 0 " .. (right - left)*200)
    player("pose pose1 1")
    if (forward - back) > 0 then
        player("pose pose2 " .. (forward - back))
    else
        player("pose pose3 " .. (back - forward))
    end

    player("pose forward 1")
    if left > right then
        player("pose left " .. (left - right))
    else
        player("pose right " .. (right - left))
    end

    --player("vely " .. (left - right))

    --[[
    if forward~=0 and back==0 then
        player("forward")
    elseif back~=0 and forward==0 then
        player("back")
    end

    if left~=0 and right==0 then
        player("left")
    elseif right~=0 and left==0 then
        player("right")
    end
    ]]
end

print("end of autoexec.lua")
