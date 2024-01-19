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

#pragma once

#include <future>
#include <memory>
#include <numeric>
#include <span>
#include <vector>

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

#include <jass/analysis/ImmutableDirectedGraph.h>

namespace jass
{
	class IAnalysis;
	class CAnalysisWorker;

	class CAnalyses: public QObject
	{
		Q_OBJECT
	public:
		CAnalyses();
		~CAnalyses();

		void AddAnalysis(std::shared_ptr<IAnalysis> analysis);

		size_t MetricCount() const;

		const QString& MetricName(size_t index) const;

		inline float MetricValue(size_t metric_index, size_t node_index) const;

		std::span<const float> MetricValues(size_t index) const;

		template <class TGraph>
		void EnqueueUpdate(const TGraph& graph);

	Q_SIGNALS:
		void MetricUpdated(const QString& name, const std::span<const float>& values);

	private Q_SLOTS:
		void OnMetricDone();
		void OnAnalysisPassComplete();

	private:
		int FindMetricIndex(const QString& name) const;
		void CancelAnalysisPass();

		void StartAnalysisPass();

		struct SMetric
		{
			QString Name;
			std::vector<float> Values;
		};

		bool m_UpdateIsPending = false;
		bool m_AnalysisPassIsInProgress = false;
		std::unique_ptr<CAnalysisWorker> m_Worker;
		std::vector<std::shared_ptr<IAnalysis>> m_Analyses;
		std::vector<SMetric> m_Metrics;
		CImmutableDirectedGraph m_PendingGraph;
		CImmutableDirectedGraph m_BusyGraph;
	};

	inline float CAnalyses::MetricValue(size_t metric_index, size_t node_index) const
	{
		const auto& metric = m_Metrics[metric_index];
		return (node_index < metric.Values.size()) ? metric.Values[node_index] : std::numeric_limits<float>::quiet_NaN();
	}

	template <class TGraph>
	void CAnalyses::EnqueueUpdate(const TGraph& graph)
	{
		if (!m_UpdateIsPending)
		{
			// Invalidate current metrics
			for (auto& metric : m_Metrics)
			{
				metric.Values.clear();
			}
		}

		m_PendingGraph.CopyView(graph);
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
}