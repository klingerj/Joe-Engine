#include <iostream>
#include "EngineInstance.h"

int RunApp() {
    try {
        JoeEngine::JEEngineInstance app = JoeEngine::JEEngineInstance();
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#include "Containers/PackedArray.h"
struct testStruct {
    int val;
    bool thing;
};

int main() {

    // packed array testing
    JoeEngine::JEPackedArray<struct testStruct> array;

    struct testStruct test1;
    test1.val = -3;
    test1.thing = true;

    array.AddElement(0, test1);
    array.RemoveElement(0);

    return RunApp();
}
