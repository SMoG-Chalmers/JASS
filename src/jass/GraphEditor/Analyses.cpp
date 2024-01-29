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

#include <thread>

#include <QtCore/QThread>

#include <jass/graphdata/GraphModelImmutableDirectedGraphAdapter.h>
#include <jass/Debug.h>
#include <jass/GraphModel.hpp>
#include "Analyses.hpp"
#include "AnalysisWorker.hpp"

#include "analyses/DepthAnalysis.h"
#include "analyses/IntegrationAnalysis.h"

namespace jass
{
	CAnalyses::CAnalyses()
		: m_Worker(new CAnalysisWorker)
	{
		connect(m_Worker.get(), &CAnalysisWorker::MetricDone, this, &CAnalyses::OnMetricDone, Qt::QueuedConnection);
		connect(m_Worker.get(), &CAnalysisWorker::AnalysisPassComplete, this, &CAnalyses::OnAnalysisPassComplete, Qt::QueuedConnection);

		AddAnalysis(std::make_shared<CDepthAnalysis>());
		AddAnalysis(std::make_shared<CIntegrationAnalysis>());
	}

	CAnalyses::~CAnalyses()
	{
	}

	void CAnalyses::AddAnalysis(std::shared_ptr<IAnalysis> analysis)
	{
		m_Analyses.push_back(std::move(analysis));
	}

	size_t CAnalyses::MetricCount() const
	{
		return m_Metrics.size();
	}

	const QString& CAnalyses::MetricName(size_t index) const
	{
		return m_Metrics[index].Name;
	}

	std::span<const float> CAnalyses::MetricValues(size_t index) const
	{
		return m_Metrics[index].Values;
	}

	void CAnalyses::OnMetricDone()
	{
		// Make sure we are on correct thread
		ASSERT(thread() == QThread::currentThread());

		if (m_UpdateIsPending)
		{
			// Metric is no longer valid
			return;
		}

		QString name;
		std::vector<float> values;
		while (m_Worker->TryGrabMetric(name, values))
		{
			int metric_index = FindMetricIndex(name);
			if (metric_index < 0)
			{
				metric_index = (int)m_Metrics.size();
				m_Metrics.push_back({ name, std::move(values) });
			}
			else
			{
				m_Worker->ReturnMetricsVector(std::move(m_Metrics[metric_index].Values));
				m_Metrics[metric_index].Values = std::move(values);
			}
			emit MetricUpdated(name, m_Metrics[metric_index].Values);
		}
	}

	void CAnalyses::OnAnalysisPassComplete(bool cancelled)
	{
		// Make sure we are on correct thread
		ASSERT(thread() == QThread::currentThread());

		ASSERT(m_AnalysisPassIsInProgress);
		m_AnalysisPassIsInProgress = false;

		if (m_UpdateIsPending)
		{
			StartAnalysisPass();
			ASSERT(!m_UpdateIsPending);
			return;
		}

		// Remove all metrics that weren't updated, since that means those are no longer applicable.
		for (size_t metric_index = 0; metric_index < m_Metrics.size(); ++metric_index)
		{
			if (m_Metrics[metric_index].Values.empty())
			{
				m_Metrics.erase(m_Metrics.begin() + metric_index);
				--metric_index;
			}
		}
	}

	void CAnalyses::EnqueueUpdate(const CGraphModel& graph_model)
	{
		if (!m_UpdateIsPending)
		{
			// Invalidate current metrics
			for (auto& metric : m_Metrics)
			{
				metric.Values.clear();
			}
		}
		m_PendingGraph.CopyView(CGraphModelImmutableDirectedGraphAdapter(graph_model));
		
		m_PendingAttributes.resize(graph_model.AttributeCount());
		for (CGraphModel::attribute_index_t i = 0; i < graph_model.AttributeCount(); ++i)
		{
			m_PendingAttributes[i].first = graph_model.AttributeName(i);
			m_PendingAttributes[i].second = graph_model.AttributeValue(i);
		}

		m_UpdateIsPending = true;

		if (m_AnalysisPassIsInProgress)
		{
			CancelAnalysisPass();
		}
		else
		{
			StartAnalysisPass();
		}
	}

	int CAnalyses::FindMetricIndex(const QString& name) const
	{
		for (int index = 0; index < m_Metrics.size(); ++index)
		{
			if (m_Metrics[index].Name == name)
			{
				return index;
			}
		}
		return -1;
	}

	void CAnalyses::CancelAnalysisPass()
	{
		m_Worker->CancelPass();
	}

	void CAnalyses::StartAnalysisPass()
	{
		// Make sure we are on correct thread
		ASSERT(thread() == QThread::currentThread());

		ASSERT(!m_AnalysisPassIsInProgress);
		m_AnalysisPassIsInProgress = true;

		ASSERT(m_UpdateIsPending);
		m_UpdateIsPending = false;
		std::swap(m_PendingGraph, m_BusyGraph);
		std::swap(m_PendingAttributes, m_BusyAttributes);

		m_Worker->BeginAnalysisPass(m_BusyGraph, m_BusyAttributes, m_Analyses);
	}
}

#include <moc_Analyses.cpp>