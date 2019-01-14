__kernel void conv2d (__global float * input,__global float * output,__global float * filter,int in_w,int k_w,int k_h,int dx,int dy ){
	int x;
	int y;
	int s_index;
	int c_index;
	int k_index = 0;
	float sum = 0.0;
	s_index = get_global_id(0) + get_global_id(1) * in_w;
	for (y = 0; y < k_h; y++) {
		c_index = s_index + y * in_w;
		for (x = 0; x < k_w; x++, k_index++) {
			sum += input[c_index + x] * filter[k_index];
		}
	}
	c_index = s_index + dy * in_w + dx;
	output[c_index] += sum;
}

__kernel void group_conv2d (__global float * input, \
							__global float * output,\
							__global float * filter,\
							int input_index,		\
							int output_index,		\
							int filter_index,		\
							int input_width,		\
							int input_height,		\
							int filter_width,		\
							int filter_heigth,		\
							int dx, int dy ){
	int x;
	int y;
	int s_index;
	int c_index;
	int k_index;
	__global float * input_p;
	__global float * output_p;
	float sum = 0.0;
	k_index = filter_index * filter_heigth * filter_width;
	input_p = &input[input_index * input_width * input_height];
	output_p = &output[output_index * input_width * input_height];
	s_index = get_global_id(0) + get_global_id(1) * input_width;
	for (y = 0; y < filter_heigth; y++) {
		c_index = s_index + y * input_width;
		for (x = 0; x < filter_width; x++, k_index++) {
			sum += input_p[c_index + x] * filter[k_index];
		}
	}
	c_index = s_index + dy * input_width + dx;
	output_p[c_index] += sum;
}

__kernel void relu (__global float * input){
	int index;
	index = get_global_id(0);
	if (input[index] < 0) {
		input[index] = 0;
	}
}

__kernel void leaky_relu (__global float * input, float rate){
	int index;
	index = get_global_id(0);
	if (input[index] < 0) {
		input[index] = rate * input[index];
	}
}

__kernel void mem_set (__global float * mem, size_t offset, float data){
	int index;
	index = get_global_id(0) + offset;
	mem[index] = data;
}


/* Do not delete or change this commit */
