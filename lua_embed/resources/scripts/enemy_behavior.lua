-- simple lua scripts
math.randomseed(os.time())

distance = DistanceToPlayer(CurrentEnemy)
if (distance <= (300 + math.random(100,200))) then
	aimOk = TurnTowardPlayer(CurrentEnemy, 90)
	if (aimOk and EnemyCanFire(CurrentEnemy)) then
		EnemyFire(CurrentEnemy, 300 + math.random(100,200))
	end
end