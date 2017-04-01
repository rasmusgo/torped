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
bind("up",   "forward=value")
bind("down", "back=value")
bind("left", "left=value")
bind("right","right=value")
bind("frame", "frame(value)")

w, s, a, d = 0, 0, 0, 0
forward, back, left, right = 0, 0, 0, 0

function frame(value)
    player("velx " .. (w - s))
    player("vely " .. (a - d))
    player("motor Kaross_Axis_F 0 " .. (forward - back)*2000 .. " 0")
    player("motor Kaross_Axis_B 0 " .. (forward - back)*2000 .. " 0")
    player("motor engine 0 0 " .. (left - right)*10)
    player("pose pose1 1")
    if (forward - back) > 0 then
        player("pose pose2 " .. (forward - back))
    else
        player("pose pose3 " .. (back - forward))
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
