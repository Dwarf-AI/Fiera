
/*! Scale layer
    It follows: 
      f(x) = s_param*x 

    where s_param is a learnable parameter 
 */

//TODO: Adding debug flags to ifdef

#pragma once
#include "layer_t.h"

#pragma pack(push, 1)
struct scale_layer_t
{
    layer_type type = layer_type::scale;
    tensor_t<float> in;
    gradient_t grads_scale; // REMEMBER: Contains sum of gradients of s_param for all examples.
    tdsize in_size;
    tdsize out_size;
    bool debug,clip_gradients_flag;
    float s_param;     /*! Single Scaler learnable parameter for all images */

    scale_layer_t(tdsize in_size,bool clip_gradients_flag = true, bool debug_flag = false)    
            // EXPECTS 2D INPUT
    /**
	* 
	* Parameters
	* ----------
	* in_size : (int m, int x, int y, int z)
	* 		Size of input matrix.
	*
	* clip_gradients_flag : bool
	* 		Whether gradients have to be clipped or not
	* 
	* debug_flag : bool
	* 		Whether to print variables for debugging purpose
	*
	**/
	
    {
        s_param = 0.001; 
        this->in_size = in_size;
        this->out_size = in_size;
        this->debug = debug_flag;    
        this->clip_gradients_flag = clip_gradients_flag;
    }

    tensor_t<float> activate(tensor_t<float> in, bool train)
	//  `activate` FORWARD PROPOGATES AND SAVES THE RESULT IN `out` VARIABLE.	    
    {
        if (train) this->in = in;

        tensor_t<float> out(in.size.m, in.size.x, 1, 1);
		
        for (int tm = 0; tm < in.size.m; tm++)
			for (int i = 0; i < in.size.x; i++){
				out(tm,i, 0, 0) = s_param * in(tm, i, 0, 0);
            }

        return out;
    }

    void fix_weights(float learning_rate)
    { 
		s_param = update_weight(s_param,grads_scale,1,false, learning_rate);
		update_gradient(grads_scale);
    }

    tensor_t<float> calc_grads(tensor_t<float>& grad_next_layer)
    {
        assert(in.size > 0);
        grads_scale.grad = 0;

        int m = grad_next_layer.size.m;

        tensor_t<float> grads_in(m, out_size.x, 1, 1);

        // summing gradients for all images
        for(int i=0; i < m; i++)
            for(int j=0; j<out_size.x; j++){
                grads_scale.grad += grad_next_layer(i,j,0,0)*in(i,j,0,0); 
                grads_in(i,j,0,0) = grad_next_layer(i,j,0,0)*s_param;
            }

        return grads_in;

    }

	void save_layer( json& model ){
		model["layers"].push_back( {
			{ "layer_type", "scale" },
			{ "in_size", {in_size.m, in_size.x, in_size.y, in_size.z} },
			{ "clip_gradients", clip_gradients_flag}
		} );
	}

    void save_layer_weight( string fileName ){
        ofstream file(fileName);
        json weights;
        weights["scale_param"] = s_param;
        file << weights;
        file.close();
    }

	void load_layer_weight(string fileName){
        ifstream file(fileName);
        json weights;
        file >> weights;
        this->s_param = weights["scale_param"];
        file.close();
	}

	void print_layer(){
		cout << "\n\n Scale Layer : \t";
		cout << "\n\t in_size:\t";
		print_tensor_size(in_size);
		cout << "\n\t out_size:\t";
		print_tensor_size(out_size);
		cout << "\n\t scale parameter:\t" << s_param;
	}


};
#pragma pack(pop)