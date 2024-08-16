#include <state.h>

State::State()
{

}

State &s() {
	static State instance;
	return &instance;
}
