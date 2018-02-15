#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <functional>
#include <unordered_map>
#include <chrono>
#include <algorithm>

#include "circuit.hpp"

// Base class - abstract interface to each library
template <typename PlaintextT, typename CiphertextT>
class Context {
	typedef std::chrono::duration<double, std::micro> microsecond;
public:
	typedef PlaintextT Plaintext;
	typedef CiphertextT Ciphertext;

	typedef std::function<microsecond(const std::list<Ciphertext>&, std::list<Ciphertext>&)> CircuitEvaluator;
	
        virtual Ciphertext encrypt(Plaintext) =0;
	virtual Plaintext decrypt(Ciphertext) =0;

	struct GateNotImplemented : public std::runtime_error {
		GateNotImplemented() : std::runtime_error("Gate not implemented.") { };
	};
	
	virtual Ciphertext Multiply(Ciphertext,Ciphertext) { throw GateNotImplemented(); };
	virtual Ciphertext Maximum(Ciphertext,Ciphertext)  { throw GateNotImplemented(); };
	virtual Ciphertext Add(Ciphertext,Ciphertext)      { throw GateNotImplemented(); };
	virtual Ciphertext Subtract(Ciphertext,Ciphertext) { throw GateNotImplemented(); };
	virtual Ciphertext Negate(Ciphertext)              { throw GateNotImplemented(); };

	virtual Ciphertext dispatch(Gate g, std::vector<Ciphertext> inputs) {
		using namespace std::placeholders;
		switch(g) {

		case(Gate::Multiply):
			return Multiply(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Maximum):
			return Maximum(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Add):
			return Add(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Subtract):
			return Subtract(inputs.at(0), inputs.at(1));
			break;

		case(Gate::Negate):
			return Negate(inputs.at(0));
			break;
		}
		throw std::runtime_error("Unknown op");
	}

	template <typename InputContainer, typename OutputContainer>
	microsecond eval(const Circuit& circ,
			 const InputContainer& input_vals,
			 OutputContainer& output_vals) {
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
			std::vector<Ciphertext> inputs;
			std::transform(assn.get_inputs().begin(),
				       assn.get_inputs().end(),
				       std::back_inserter(inputs),
				       [&eval_map](Wire w) {
					       // throws out_of_range if not present in the map
					       return eval_map.at(w.get_name());
				       });
			Ciphertext output = dispatch(assn.get_op(), inputs);
			eval_map.insert({assn.get_output().get_name(), output});
		}

		auto end_time = high_res_clock::now();
		microsecond duration = microsecond(end_time - start_time);

		// Look up the required outputs in the eval_map and
		// push them onto output_vals.
		auto output_wires_it = circ.get_outputs().begin();
		auto output_wires_end = circ.get_outputs().end();
		for (; output_wires_it != output_wires_end; ++output_wires_it) {
			output_vals.push_back(eval_map.at(output_wires_it->get_name()));
		}
		return duration;
	}
	
	virtual CircuitEvaluator compile(const Circuit& circ) {
		using std::placeholders::_1;
		using std::placeholders::_2;
		auto run = std::bind(&Context::eval<std::list<Ciphertext>, std::list<Ciphertext> >, this, circ, _1, _2);
		return CircuitEvaluator(run);
	}
};

#endif // CONTEXT_HPP
