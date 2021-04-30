#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <hilti/rt/exception.h>
#include <hilti/rt/init.h>
#include <hilti/rt/types/reference.h>
#include <hilti/rt/types/stream.h>

#include <spicy/rt/init.h>
#include <spicy/rt/parser.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    static const spicy::rt::Parser* parser = nullptr;

    if ( ! parser ) {
        hilti::rt::init();
        spicy::rt::init();

        for ( auto* p : spicy::rt::parsers() )
            parser = p;
    }

    assert(parser);
    assert(parser->parse1);

    hilti::rt::ValueReference<hilti::rt::Stream> stream;
    stream->append(reinterpret_cast<const char*>(Data), Size);

    try {
        (*parser->parse1)(stream, {}, {});
    } catch ( const spicy::rt::ParseError& ) {
    } catch ( const hilti::rt::StackSizeExceeded& ) { // FIXME(bbannier): should we trigger this on small inputs?
    }

    return 0; // Non-zero return values are reserved for future use.
}

extern "C" int LLVMFuzzerRunDriver(int* argc, char*** argv, int (*UserCb)(const uint8_t* Data, size_t Size));

// We provide our own `main` to avoid linking to hilti-rt's weak `main` symbol.
int main(int argc, char** argv) { LLVMFuzzerRunDriver(&argc, &argv, LLVMFuzzerTestOneInput); }
