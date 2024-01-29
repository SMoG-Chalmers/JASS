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

#include <stdexcept>
#include <vector>

#include <QtGui/QImage>

#include "../math/Gaussian.h"

namespace jass
{
	QRgb Blend(const QRgb& c0, const QRgb& c1, uint8_t alpha)
	{
		const uint8_t inv_alpha = 255 - alpha;
		return qRgba(
			(inv_alpha * qRed(c0)   + alpha * qRed(c1))   / 255,
			(inv_alpha * qGreen(c0) + alpha * qGreen(c1)) / 255,
			(inv_alpha * qBlue(c0)  + alpha * qBlue(c1))  / 255,
			(inv_alpha * qAlpha(c0) + alpha * qAlpha(c1)) / 255);
	}

	QRgb AddOver(QRgb over, QRgb under)
	{
		const unsigned char combined_alpha = qAlpha(over) + ((255 - qAlpha(over)) * qAlpha(under)) / 255;
		if (!combined_alpha)
			return 0;
		const unsigned char t = (255 * (combined_alpha - qAlpha(over))) / combined_alpha;
		const unsigned char tinv = 255 - t;
		return qRgba(
			(tinv * qRed(over)   + t * qRed(under))   / 255,
			(tinv * qGreen(over) + t * qGreen(under)) / 255,
			(tinv * qBlue(over)  + t * qBlue(under))  / 255,
			combined_alpha);
	}

	inline constexpr QRgb qSetAlpha(QRgb color, unsigned char alpha)
	{
		return (color & 0xFFFFFF) | ((unsigned int)alpha << 24);
	}

	void DropShadow(QImage& img, QColor color, unsigned int radius, int offs_x, int offs_y, float sigma_range)
	{
		using namespace std;

		if (img.format() != QImage::Format_ARGB32)
		{
			throw std::runtime_error("Unsupported image format");
		}

		auto* blur_kernel = (float*)alloca((radius + 1) * sizeof(float));
		GenerateGaussianKernel(sigma_range, std::span<float>(blur_kernel, radius + 1));

		std::vector<float> tmp(img.width() * img.height());

		unsigned char* const bits = img.bits();
		const unsigned int   stride = img.bytesPerLine();

		// Vertical pass
		auto* dst = tmp.data();
		for (int y = 0; y < (int)img.height(); ++y)
		{
			const int y_min = -min(y - offs_y, (int)radius);
			const int y_max = min((int)img.height() - y - 1 + offs_y, (int)radius);
			for (int x = 0; x < (int)img.width(); ++x)
			{
				float s = 0.0f;
				for (int i = y_min; i <= y_max; ++i)
				{
					const auto alpha = bits[(y - offs_y + i) * stride + (x * 4) + 3];
					s += blur_kernel[abs(i)] * (1.0f / 255.0f) * alpha;
				}
				*dst++ = s;
			}
		}

		QRgb shadow_color = color.rgba();
		const float opacity = (float)qAlpha(shadow_color) / 255.0f;

		// Horizontal pass
		const auto* src = tmp.data();
		for (int y = 0; y < (int)img.height(); ++y)
		{
			for (int x = 0; x < (int)img.width(); ++x)
			{
				const int x_min = -min(x - offs_x, (int)radius);
				const int x_max = min((int)img.width() - x - 1 + offs_x, (int)radius);
				float s = 0.0f;
				for (int i = x_min; i <= x_max; ++i)
					s += blur_kernel[abs(i)] * (*(src + i - offs_x));

				shadow_color = qSetAlpha(shadow_color, (unsigned char)(0.5f + 255.0f * (s * opacity)));
				auto& color = *(QRgb*)&bits[y * stride + (x * 4)];
				color = AddOver(color, shadow_color);
				++src;
			}
		}
	}
}