typedef const char* (*PrintFunc)(unsigned int value, void*data);
const char* print_format(unsigned int value, void*format);
const char* print_float(unsigned int value, void*format);

typedef struct {
	unsigned int id;
	const char* name;
} data_store;

typedef struct {
	int lowbit;
	int highbit;

	PrintFunc func;
	void *data;
} data_print_store;

typedef struct {
	unsigned int offset;
	const char* name;

	// max 32 bits, so max 32 subfields
	data_print_store data[32];
} object_field_store;

typedef struct {
	unsigned int id;
	const char *name;

	// You can freely increase this array size, if it's full
	object_field_store fields[100];
} object_store;

unsigned int find_object_type(unsigned int name);
char *format_load(int object_type, int offset, unsigned int value);

//required to be defined elesewhere:
extern data_print_store user_data_print[];
