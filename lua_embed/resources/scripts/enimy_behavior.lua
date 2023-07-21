-- simple lua scripts
math.randomseed(os.time())

distance = DistanceToPlayer(CurrentEnimy)
if (distance <= (300 + math.random(100,200))) then
	aimOk = TurnTwardPlayer(CurrentEnimy, 90)
	if (aimOk and EnimyCanFire(CurrentEnimy)) then
		EnimyFire(CurrentEnimy, 300 + math.random(100,200))
	end
end