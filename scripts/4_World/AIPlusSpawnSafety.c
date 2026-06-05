class AIPlusSpawnSafety
{
	static protected const float MAX_PLACEMENT_TERRAIN_DELTA = 2.0;

	static bool FindSafeAIPosition(vector requestedPosition, out vector safePosition, float searchMin = 2.0, float searchMax = 25.0, int attempts = 40)
	{
		if (GetSafeAIPosition(requestedPosition, safePosition))
			return true;

		for (int i = 0; i < attempts; i++)
		{
			vector candidate = ExpansionMath.GetRandomPointInRing(requestedPosition, searchMin, searchMax);
			if (GetSafeAIPosition(candidate, safePosition))
				return true;
		}

		return false;
	}

	static bool GetSafeAIPosition(vector requestedPosition, out vector safePosition)
	{
		if (!GetGame() || requestedPosition == vector.Zero)
			return false;

		vector terrainPosition = requestedPosition;
		terrainPosition[1] = GetGame().SurfaceY(terrainPosition[0], terrainPosition[2]);

		if (IsWaterPosition(terrainPosition))
			return false;

		vector placement = ExpansionAIPatrol.GetPlacementPosition(terrainPosition);
		if (placement == vector.Zero)
			return false;

		float terrainY = GetGame().SurfaceY(placement[0], placement[2]);
		if (Math.AbsFloat(placement[1] - terrainY) > MAX_PLACEMENT_TERRAIN_DELTA)
			return false;

		safePosition = Vector(placement[0], terrainY, placement[2]);
		if (IsWaterPosition(safePosition))
			return false;

		if (IsObstructedForAI(safePosition))
			return false;

		return true;
	}

	protected static bool IsWaterPosition(vector position)
	{
		if (GetGame().SurfaceIsSea(position[0], position[2]))
			return true;

		if (GetGame().SurfaceIsPond(position[0], position[2]))
			return true;

		return GetGame().GetWaterDepth(position) > 0;
	}

	protected static bool IsObstructedForAI(vector position)
	{
		array<Object> excludedObjects = {};
		array<Object> collidedObjects = {};
		vector collisionCenter = position + Vector(0, 1.0, 0);
		vector collisionSize = Vector(0.8, 1.8, 0.8);

		return GetGame().IsBoxCollidingGeometry(collisionCenter, Vector(0, 0, 0), collisionSize, ObjIntersectView, ObjIntersectGeom, excludedObjects, collidedObjects);
	}
}
