#define INIT_PAYINFO_SIZE 4
#define INIT_RTRINFO_SIZE 12


struct __attribute__((__packed__)) INIT_PAYINFO
{
	uint16_t number;
    uint16_t upi;
};
struct __attribute__((__packed__)) INIT_RTRINFO
{
	uint16_t rtrid;
    uint16_t rtrport;
	uint16_t dataport;
	uint16_t cost;
	uint32_t rtrip;
};

void init_handler(char *cntrl_payload);
