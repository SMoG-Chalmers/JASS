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
	class CGraphModel;

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

		void EnqueueUpdate(const CGraphModel& graph_model);

		int FindMetricIndex(const QString& name) const;

	Q_SIGNALS:
		void MetricUpdated(const QString& name, const std::span<const float>& values);

	private Q_SLOTS:
		void OnMetricDone();
		void OnAnalysisPassComplete(bool cancelled);

	private:
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
		std::vector<std::pair<QString, QVariant>> m_PendingAttributes;
		CImmutableDirectedGraph m_BusyGraph;
		std::vector<std::pair<QString, QVariant>> m_BusyAttributes;
	};

	inline float CAnalyses::MetricValue(size_t metric_index, size_t node_index) const
	{
		const auto& metric = m_Metrics[metric_index];
		return (node_index < metric.Values.size()) ? metric.Values[node_index] : std::numeric_limits<float>::quiet_NaN();
	}
}