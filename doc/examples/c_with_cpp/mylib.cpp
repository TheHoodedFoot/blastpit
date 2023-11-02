#include <iostream>

        void myfunction(void);

        void myfunction(void)
        {
                std::cout << "This uses std::cout" << std::endl;
        }

extern "C" {

        void cfunction(void);

        void cfunction(void)
        {
                myfunction();
        }
}
