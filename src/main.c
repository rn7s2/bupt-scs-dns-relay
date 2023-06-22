#include "argsparser.h"

int main(int argc, char **argv)
{
    struct config config = {0};
    parse_args(argc, argv, &config);
    if (config.debug_level) {
        dump_args(&config);
    }

    return 0;
}
