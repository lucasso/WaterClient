#ifndef _LUCASSO_OPTION_H_
#define _LUCASSO_OPTION_H_

struct EmptyOptionException {};

template <class T>
class Option
{
	T value;
	bool exists;

public:

	Option() : exists(false) {}
	Option(T valueArg) : value(valueArg), exists(true) {}

	T getValue() const
	{
		if (this->exists) return this->value;
		return T();
	}

	bool hasSome() const { return this->exists; }
	bool isEmpty() const { return ! this->exists; }
};


#endif /* _LUCASSO_OPTION_H_ */
