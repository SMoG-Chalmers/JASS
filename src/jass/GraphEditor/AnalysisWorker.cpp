/*
Copyright Ioanna Stavroulaki 2023

This file is part of JASS.

JASS is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version.

JASS is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along 
with JASS. If not, see <https://www.gnu.org/licenses/>.
*/

#include <jass/Debug.h>
#include "AnalysisWorker.hpp"

namespace jass
{
	CAnalysisWorker::CAnalysisWorker() {}
	
	CAnalysisWorker::~CAnalysisWorker() {}

	void CAnalysisWorker::BeginAnalysisPass(const CImmutableDirectedGraph& graph, const std::vector<std::pair<QString, QVariant>>& graph_attributes, std::span<std::shared_ptr<IAnalysis>> analyses)
	{
		ASSERT(!Busy());

		m_Cancelled = false;

		m_Graph = &graph;
		m_GraphAttributes = &graph_attributes;

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

	bool CAnalysisWorker::TryGetGraphAttribute(const QString& name, QVariant& out_value) const
	{
		for (const auto& a : *m_GraphAttributes)
		{
			if (a.first == name)
			{
				out_value = a.second;
				return true;
			}
		}
		return false;
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

		emit AnalysisPassComplete(m_Cancelled);
	}
}

#include <moc_AnalysisWorker.cpp>