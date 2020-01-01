/*
 *  tslib/src/ts_getxy.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * $Id: testutils.c,v 1.2 2004/10/19 22:01:27 dlowder Exp $
 *
 * Waits for the screen to be touched, averages x and y sample
 * coordinates until the end of contact
 */
#include <stdio.h>
#include <stdlib.h>
#include "tslib.h"


static int sort_by_x(const void* a, const void *b)
{
	return (((struct ts_sample *)a)->x - ((struct ts_sample *)b)->x);
}

static int sort_by_y(const void* a, const void *b)
{
	return (((struct ts_sample *)a)->y - ((struct ts_sample *)b)->y);
}

/*

	У׼ʱ��ȡ���㣬
	ѹ����Ϊ0��ʼ����������ǰ��128����
	ѹ��Ϊ0���˳���
	ѹ��ֵӦ������٣�

*/
int getxy(struct tsdev *ts, int *x, int *y)
{

#define MAX_SAMPLES 128

	struct ts_sample samp[MAX_SAMPLES];
	int index, middle;
	int ret;
	
	/*У׼��ʱ��ֱ��ʹ��raw����Ҳ����û�о���������ԭʼ����*/
	samp[0].pressure = 0;
	
	do {
		ret = ts_read_raw(ts, &samp[0], 1);
		if (ret < 0) 
		{
			printf("read err\r\n");
			return -1;
		}
		else if(ret == 1)
		{
			if(samp[0].pressure > 0)
			{
				printf("read press %d\r\n", samp[0].pressure);
				break;
			}
		}
	} while (1);
		
	printf("get xy pen down\r\n");
	/* Now collect up to MAX_SAMPLES touches into the samp array. */
	index = 0;
	do {
		ret = ts_read_raw(ts, &samp[index], 1);
		
		if ( ret < 0) 
		{
			printf("read err\r\n");
			return -1;
		}
		else if(ret == 1)
		{

			if(samp[index].pressure <= 0)
			{
				printf("read press 0\r\n");
				break;
			}
			
			if (index < MAX_SAMPLES-1)
				index++;// ���������˲���������
		}
		/*
			��ֲ��STM32ʱ�����и�BUG��STM32 ��ts_read_raw���п��ܶ��������ݣ�ת���ٶȲ�����
			�޸�Ϊ��������������ݣ���.
		*/
	} while (1);

	printf("get xy pen up\r\n");
	printf("Took %d samples...\r\n",index);

	/*
	 * At this point, we have samples in indices zero to (index-1)
	 * which means that we have (index) number of samples.  We want
	 * to calculate the median of the samples so that wild outliers
	 * don't skew the result.  First off, let's assume that arrays
	 * are one-based instead of zero-based.  If this were the case
	 * and index was odd, we would need sample number ((index+1)/2)
	 * of a sorted array; if index was even, we would need the
	 * average of sample number (index/2) and sample number
	 * ((index/2)+1).  To turn this into something useful for the
	 * real world, we just need to subtract one off of the sample
	 * numbers.  So for when index is odd, we need sample number
	 * (((index+1)/2)-1).  Due to integer division truncation, we
	 * can simplify this to just (index/2).  When index is even, we
	 * need the average of sample number ((index/2)-1) and sample
	 * number (index/2).  Calculate (index/2) now and we'll handle
	 * the even odd stuff after we sort.
	 */
	middle = index/2;
	if (x) 
	{
		/*   ʹ��qsort ����������*/
		qsort(samp, index, sizeof(struct ts_sample), sort_by_x);
		if (index & 1)
			*x = samp[middle].x;
		else
			*x = (samp[middle-1].x + samp[middle].x) / 2;
	}
	
	if (y) 
	{
		qsort(samp, index, sizeof(struct ts_sample), sort_by_y);
		if (index & 1)
			*y = samp[middle].y;
		else
			*y = (samp[middle-1].y + samp[middle].y) / 2;
	}
	
	return 0;
}

void ts_flush (struct tsdev *ts)
{
	/* Read all unread touchscreen data, 
	 * so that we are sure that the next data that we read
	 * have been input after this flushing.
	 */
 
}
