#pragma once

void rgb2hsv(const unsigned char src_r, const unsigned char src_g, const unsigned char src_b, unsigned char *dst_h,
             unsigned char *dst_s, unsigned char *dst_v);
void hsv2rgb(const unsigned char src_h, const unsigned char src_s, const unsigned char src_v, unsigned char *dst_r,
             unsigned char *dst_g, unsigned char *dst_b);