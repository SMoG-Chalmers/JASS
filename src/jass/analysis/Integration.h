#pragma once

namespace jass
{
	// N  = Number of reached nodes INCLUDING origin node
	// TD = Total depth
	float CalculateIntegrationScore(unsigned int N, float TD, float& out_MD, float& out_RA, float& out_RRA);
}