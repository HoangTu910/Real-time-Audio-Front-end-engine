extern int main();

void Reset_Handler(void)
{
    main();
    while (1);
}

__attribute__((section(".isr_vector")))
void (*vector_table[])(void) = {
    (void (*)(void))0x20010000,
    Reset_Handler
};