/*
 © BV, 2010
mailto:borisvorontsov@gmail.com
*/

//Фильтр "Размытие"
//функция потока OpenCL, рассчитанная  на выполнение на GPU
__kernel void Blur(__global uchar4* InputImage, __global uchar4* OutputImage, const uint Width, const uint Height, const uint Level)
{
	//Получаем наш столбец в растре (на каждый столбец по потоку в группе)
	int x = get_global_id(0);
	
	int x1, y1;
	int x2, y2;
	int x3, y3;
	float4 pixelBGRA;
	uint r, g, b;
	uint pixels;
	
	//Идем по всему столбцу и выполняем размытие...
	for (int y = 0; y < Height; y++)
	{
		int pos0 = x + y * Width;

		x1 = x - (Level >> 1);
		if (x1 < 0) x1 = 0;
		x2 = x + (Level >> 1);
		if (x2 > (Width - 1)) x2 = (Width - 1);
		
		y1 = y - (Level >> 1);
		if (y1 < 0) y1 = 0;
		y2 = y + (Level >> 1);
		if (y2 > (Height - 1)) y2 = (Height - 1);
		
		r = 0;
		g = 0;
		b = 0;
		
		//Суммируем компоненты пикселей в заданной уровнем (Level) зоне
		for (x3 = x1; x3 <= x2; x3++)
		{
			for (y3 = y1; y3 <= y2; y3++)
			{
				int pos1 = x3 + y3 * Width;
				pixelBGRA = (float4)(InputImage[pos1].x, InputImage[pos1].y, InputImage[pos1].z, InputImage[pos1].w);
				
				b += pixelBGRA.x;
				g += pixelBGRA.y;
				r += pixelBGRA.z;
			}
		}
		
		//...и выполняем устреднение
		pixels = (x2 - x1 + 1) * (y2 - y1 + 1);
		r /= pixels;
		g /= pixels;
		b /= pixels;
		
		//Присваиваем новый пиксель итоговому растру
		OutputImage[pos0] = (uchar4)(b, g, r, pixelBGRA.w);
	}
}
