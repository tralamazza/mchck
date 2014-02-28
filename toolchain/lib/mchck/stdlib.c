#include <mchck.h>

int
atoi(const char* str)
{
	return strtol(str, NULL, 10);
}

long int
strtol(const char* str, char** endptr, int base)
{
	int r = 0, s = 1;
	if (str == NULL || base == 1 || base > 36) {
		goto end;
	}
	int i = 0;
	// eat all ws
	for (; str[i] == ' '; i++) {
	}
	// signed?
	if (str[i] == '-') {
		s = -1;
		i++;
	}
	// prefixed?
	if (str[i] == '0') {
		i++;
		if (str[i] == 'x' || str[i] == 'X') {
			if (base == 0) {
				base = 16;
			}
			i++;
		} else if (base == 0) {
			base = 8;
		}
	}

	int k = i; // mark current position
	if (base <= 10) {
		// base up to 10, valid digits: 0 .. 0 + base - 1
		while (str[i] >= '0' && str[i] < ('0' + base)) {
			r = (r * base) + (str[i++] - '0');
		}
	} else {
		// bases from 10 up to 36
		int j = -1; // marker for progress
		int rbase = base - 10; // base remainder
		while (i != j) { // stop if no progress was made
			j = i;
			// all digits 0..9
			while (str[i] >= '0' && str[i] <= '9') {
				r = (r * base) + (str[i++] - '0');
			}
			// letters from 'A' .. 'A' + base - 10
			while (str[i] >= 'A' && str[i] < ('A' + rbase)) {
				r = (r * base) + (str[i++] - 'A' + 10);
			}
			// letters from 'a' .. 'a' + base - 10
			while (str[i] >= 'a' && str[i] < ('a' + rbase)) {
				r = (r * base) + (str[i++] - 'a' + 10);
			}
		}
	}
	// did we move fwd?
	if (i > k) {
		str = &str[i];
	}
end:
	if (endptr != NULL) {
		*endptr = (char*)str;
	}
	return s * r;
}
