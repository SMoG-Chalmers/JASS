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

#include <jass/utils/bitvec.h>
#include "../GraphTool.h"

namespace jass
{
	class CCategorySpriteSet;

	class CNodeTool : public CGraphTool
	{
	public:
		CNodeTool();
		~CNodeTool();

		void SetSpriteSet(std::shared_ptr<CCategorySpriteSet> sprites);

		void Activate(const SGraphToolContext& ctx) override;
		void Deactivate() override;
		void Paint(QPainter& painter, const QRect& rc) override;
		void leaveEvent(QEvent& event) override;
		void mouseMoveEvent(QMouseEvent& event) override;
		void mousePressEvent(QMouseEvent& event) override;
		void wheelEvent(QWheelEvent& event) override;
		void keyPressEvent(QKeyEvent& event) override;

	private:
		void CheckCategory();
		void SetCategory(int category);
		void ShowStamp(const QPoint& pt);
		void HideStamp();
		void SetHoverNode(CGraphLayer::element_t node);

		CNodeGraphLayer* m_NodeLayer = nullptr;
		CGraphLayer::element_t m_HoverNode = CGraphLayer::NO_ELEMENT;
		bitvec m_TempBitVec;
		int m_CurrentCategory = 0;
		bool m_Stamping = false;
		QPoint m_StampPos;
		std::shared_ptr<CCategorySpriteSet> m_Sprites;
	};
}