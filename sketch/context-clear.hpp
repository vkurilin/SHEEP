#ifndef CONTEXT_CLEAR_HPP
#define CONTEXT_CLEAR_HPP

#include <unordered_map>
#include <chrono>

#include "circuit.hpp"
#include "context.hpp"

// struct Wire_container {
// 	const Wire& w;
// 	Wire_container(const Wire& w_) : w(w_) {};
// };

class ContextClear : public Context<bool, bool> {
public:
	Ciphertext encrypt(Plaintext p) {
		return p; // both Plaintext and Ciphertext are typedef'd to bool
	}

	Plaintext decrypt(Ciphertext c) {
		return c;
	}
	
	Ciphertext And(Ciphertext a, Ciphertext b) {
		return a & b;
	}

	Ciphertext Or(Ciphertext a, Ciphertext b) {
		return a | b;
	}

	Ciphertext Xor(Ciphertext a, Ciphertext b) {
		return a != b;
	}
	
	GateFn get_op(Gate g) {
		using namespace std::placeholders;
		switch(g) {
		case(Gate::And):
			return GateFn(std::bind(&ContextClear::And, this, _1, _2));
			break;

		case(Gate::Or):
			return GateFn(std::bind(&ContextClear::Or, this, _1, _2));
			break;

		case(Gate::Xor):
			return GateFn(std::bind(&ContextClear::Xor, this, _1, _2));
			break;

		}
		throw std::runtime_error("Unknown op");
	}

	// each Context concrete class provides its own eval method,
	// and also a compile method which can perform any
	// library-specific optimization.  Here we only provide 'eval'
	// and rely on the default implementation of compile in the
	// base Context class.
	std::chrono::duration<double, std::micro>
	eval(const Circuit& circ,
	     const std::list<Ciphertext>& input_vals,
	     std::list<Ciphertext>& output_vals) {
		std::unordered_map<std::string, Ciphertext> eval_map;

		// add Circuit::inputs and inputs into the map
		auto input_vals_it = input_vals.begin();
		auto input_wires_it = circ.get_inputs().begin();
		const auto input_wires_end = circ.get_inputs().end();
		for (; input_vals_it != input_vals.end() || input_wires_it != input_wires_end;
		     ++input_vals_it, ++input_wires_it)
		{
			eval_map.insert({input_wires_it->get_name(), *input_vals_it});
		}

		// error check: both iterators should be at the end
		if (input_vals_it != input_vals.end() || input_wires_it != input_wires_end)
			throw std::runtime_error("Number of inputs doesn't match");


		// This is where the actual evaluation occurs.  For
		// each assignment, look up the input Wires in the
		// map, insert the output wire (of the gate) with the
		// required name into eval_map.

		typedef std::chrono::duration<double, std::micro> microsecond;
		typedef std::chrono::high_resolution_clock high_res_clock;
		auto start_time = high_res_clock::now();
		
		for (const Assignment assn : circ.get_assignments()) {
			// throws out_of_range if not present in the map
			Ciphertext input1 = eval_map.at(assn.get_input1().get_name());
			Ciphertext input2 = eval_map.at(assn.get_input2().get_name());
			auto op = get_op(assn.get_op());
			Ciphertext output = op(input1, input2);
			eval_map.insert({assn.get_output().get_name(), output});
		}
		
		auto end_time = high_res_clock::now();
		microsecond duration = microsecond(end_time - start_time);

		
		// Look up the required outputs in the eval_map and
		// push them onto output_vals.
		auto output_wires_it = circ.get_outputs().begin();
		auto output_wires_end = circ.get_outputs().end();
		for (; output_wires_it != output_wires_end; ++output_wires_it) {
			output_vals.push_back(eval_map.at(output_wires_it->wire.get_name()));
		}
				
		return duration;
	}
};

#endif // CONTEXT_CLEAR_HPP
