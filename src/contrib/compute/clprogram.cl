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

__kernel void mem_dump (__global float * src, __global float * dst){
	int index;
	index = get_global_id(0);
	dst[index] = src[index];
}

__kernel void mem_set (__global float * mem, float data){
	int index;
	index = get_global_id(0);
	mem[index] = data;
}


/* Do not delete or change this commit */
