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
class testStruct {
public:
    int val;
    bool thing;
};

int main() {

    // packed array testing
    JoeEngine::PackedArray<testStruct> array;

    testStruct test1;
    test1.val = 1;
    test1.thing = true;

    testStruct test2;
    test2.val = 2;
    test2.thing = true;

    testStruct test3;
    test3.val = 3;
    test3.thing = false;

    testStruct test4;
    test4.val = 4;
    test4.thing = true;

    testStruct test5;
    test5.val = 5;
    test5.thing = false;

    array.AddElement(0, test1);
    //array.RemoveElement(0);
    array.AddElement(1, test2);
    //array.RemoveElement(1);
    array.AddElement(2, test3);
    //array.RemoveElement(2);
    array.AddElement(3, test4);
    //array.RemoveElement(3);
    array.AddElement(4, test5);

    array[4] = test1;
    array[4] = array[4];

    for (auto& ele : array) {
        std::cout << ele.val << std::endl;
    }

    return RunApp();
}
