/*
Copyright XMN Software AB 2023

JASS is free software: you can redistribute it and/or modify it under the
terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version. The GNU Lesser General Public License is intended to
guarantee your freedom to share and change all versions of a program --
to make sure it remains free software for all its users.

JASS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with JASS. If not, see <http://www.gnu.org/licenses/>.
*/

#include <jass/Debug.h>
#include "AnalysisWorker.hpp"

namespace jass
{
	CAnalysisWorker::CAnalysisWorker() {}
	
	CAnalysisWorker::~CAnalysisWorker() {}

	void CAnalysisWorker::BeginAnalysisPass(const CImmutableDirectedGraph& graph, std::span<std::shared_ptr<IAnalysis>> analyses)
	{
		ASSERT(!Busy());

		m_Cancelled = false;

		m_Graph = &graph;

		m_Analyses.clear();
		for (auto& analysis : analyses)
		{
			m_Analyses.push_back(analysis);
		}

		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			while (!m_Metrics.empty())
			{
				m_FreeMetricVectors.push_back(std::move(m_Metrics.back().Values));
				m_Metrics.pop_back();
			}
		}

		m_AnalysisPassResult = std::async(std::launch::async, &CAnalysisWorker::AnalysisThread, this);
	}

	void CAnalysisWorker::CancelPass()
	{
		m_Cancelled = true;
	}

	bool CAnalysisWorker::TryGrabMetric(QString& out_name, std::vector<float>& out_values)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);

		if (m_Metrics.empty())
		{
			return false;
		}

		out_name = std::move(m_Metrics.back().Name);
		out_values = std::move(m_Metrics.back().Values);
		m_Metrics.pop_back();

		return true;
	}

	void CAnalysisWorker::ReturnMetricsVector(std::vector<float>&& v)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_FreeMetricVectors.push_back(std::move(v));
	}

	const CImmutableDirectedGraph& CAnalysisWorker::ImmutableDirectedGraph() const
	{
		return *m_Graph;
	}

	std::vector<float> CAnalysisWorker::NewMetricVector()
	{
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			if (!m_FreeMetricVectors.empty())
			{
				auto v = std::move(m_FreeMetricVectors.back());
				m_FreeMetricVectors.pop_back();
				v.clear();
				return std::move(v);
			}
		}

		return {};
	}

	void CAnalysisWorker::OutputMetric(const QString& name, std::vector<float>&& values)
	{
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Metrics.push_back({ name, std::move(values) });
		}
		
		emit MetricDone();
	}

	void CAnalysisWorker::AnalysisThread()
	{
		for (auto& analysis : m_Analyses)
		{
			if (m_Cancelled)
			{
				break;
			}

			analysis->RunAnalysis(*this);
		}

		m_Graph = nullptr;

		emit AnalysisPassComplete();
	}
}

#include <moc_AnalysisWorker.cpp>