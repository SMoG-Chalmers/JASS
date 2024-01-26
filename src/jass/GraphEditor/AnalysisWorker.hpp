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

#include <memory>
#include <span>
#include <vector>
#include <future>
#include <mutex>

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

#include "Analysis.h"

namespace jass
{
	class CImmutableDirectedGraph;

	class CAnalysisWorker: public QObject, public IAnalysisContext
	{
		Q_OBJECT
	public:
		CAnalysisWorker();
		~CAnalysisWorker();

		void BeginAnalysisPass(const CImmutableDirectedGraph& graph, const std::vector<std::pair<QString, QVariant>>& graph_attributes, std::span<std::shared_ptr<IAnalysis>> analyses);

		void CancelPass();

		bool TryGrabMetric(QString& out_name, std::vector<float>& out_values);

		void ReturnMetricsVector(std::vector<float>&& v);

		// IAnalysisContext
		const CImmutableDirectedGraph& ImmutableDirectedGraph() const override;
		bool TryGetGraphAttribute(const QString& name, QVariant& out_value) const override;
		std::vector<float> NewMetricVector() override;
		void OutputMetric(const QString& name, std::vector<float>&& values) override;

	Q_SIGNALS:
		void MetricDone();
		void AnalysisPassComplete(bool cancelled);

	private:
		inline bool Busy() const { return nullptr != m_Graph; }
		
		void AnalysisThread();

		struct SMetric
		{
			QString Name;
			std::vector<float> Values;
		};

		const CImmutableDirectedGraph* m_Graph = nullptr;
		const std::vector<std::pair<QString, QVariant>>* m_GraphAttributes = nullptr;
		std::vector<std::shared_ptr<IAnalysis>> m_Analyses;
		std::vector<SMetric> m_Metrics;
		std::future<void> m_AnalysisPassResult;
		std::mutex m_Mutex;
		bool m_Cancelled = false;

		std::vector<std::vector<float>> m_FreeMetricVectors;
	};
}