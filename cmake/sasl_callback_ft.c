#include <sasl/sasl.h>
#include <sasl/saslplug.h>

int callback(void) {
	return 0;
}

int main() {
	(sasl_callback_ft *)callback;
	return 0;
}
