/* generated vector source file - do not edit */
        #include "bsp_api.h"
        /* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
        #if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_MAX_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_b_uart_rxi_isr, /* SCI0 RXI (Receive data full) */
            [1] = sci_b_uart_txi_isr, /* SCI0 TXI (Transmit data empty) */
            [2] = sci_b_uart_tei_isr, /* SCI0 TEI (Transmit end) */
            [3] = sci_b_uart_eri_isr, /* SCI0 ERI (Receive error) */
            [4] = comp_hs_int_isr, /* ACMPHS0 INT (High Speed Comparator channel 0 interrupt) */
            [5] = gpt_capture_compare_a_isr, /* GPT0 CAPTURE COMPARE A (Capture/Compare match A) */
            [6] = adc_b_limclpi_isr, /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
            [7] = adc_b_err0_isr, /* ADC ERR0 (A/D converter unit 0 Error) */
            [8] = adc_b_err1_isr, /* ADC ERR1 (A/D converter unit 1 Error) */
            [9] = adc_b_resovf0_isr, /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
            [10] = adc_b_resovf1_isr, /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
            [11] = adc_b_calend0_isr, /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
            [12] = adc_b_calend1_isr, /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
            [13] = adc_b_adi0_isr, /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
            [14] = adc_b_adi1_isr, /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
            [15] = adc_b_adi2_isr, /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
            [16] = adc_b_adi3_isr, /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
            [17] = adc_b_adi4_isr, /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
            [18] = adc_b_adi5678_isr, /* ADC ADI5678 (End of A/D scanning operation(Gr.5678)) */
            [19] = adc_b_fifoovf_isr, /* ADC FIFOOVF (FIFO data overflow) */
            [20] = adc_b_fiforeq0_isr, /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
            [21] = adc_b_fiforeq1_isr, /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
            [22] = adc_b_fiforeq2_isr, /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
            [23] = adc_b_fiforeq3_isr, /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
            [24] = adc_b_fiforeq4_isr, /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
            [25] = adc_b_fiforeq5678_isr, /* ADC FIFOREQ5678 (FIFO data read request interrupt(Gr.5678)) */
            [26] = gpt_capture_compare_b_isr, /* GPT4 CAPTURE COMPARE B (Capture/Compare match B) */
            [27] = gpt_capture_compare_a_isr, /* GPT5 CAPTURE COMPARE A (Capture/Compare match A) */
            [28] = gpt_capture_compare_b_isr, /* GPT5 CAPTURE COMPARE B (Capture/Compare match B) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_MAX_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI0_RXI,GROUP0), /* SCI0 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI0_TXI,GROUP1), /* SCI0 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI0_TEI,GROUP2), /* SCI0 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI0_ERI,GROUP3), /* SCI0 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_ACMPHS0_INT,GROUP4), /* ACMPHS0 INT (High Speed Comparator channel 0 interrupt) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_GPT0_CAPTURE_COMPARE_A,GROUP5), /* GPT0 CAPTURE COMPARE A (Capture/Compare match A) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_ADC12_LIMCLPI,GROUP6), /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_ADC12_ERR0,GROUP7), /* ADC ERR0 (A/D converter unit 0 Error) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_ADC12_ERR1,GROUP0), /* ADC ERR1 (A/D converter unit 1 Error) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_ADC12_RESOVF0,GROUP1), /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_ADC12_RESOVF1,GROUP2), /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_ADC12_CALEND0,GROUP3), /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_ADC12_CALEND1,GROUP4), /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_ADC12_ADI0,GROUP5), /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_ADC12_ADI1,GROUP6), /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_ADC12_ADI2,GROUP7), /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
            [16] = BSP_PRV_VECT_ENUM(EVENT_ADC12_ADI3,GROUP0), /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
            [17] = BSP_PRV_VECT_ENUM(EVENT_ADC12_ADI4,GROUP1), /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
            [18] = BSP_PRV_VECT_ENUM(EVENT_ADC12_ADI5678,GROUP2), /* ADC ADI5678 (End of A/D scanning operation(Gr.5678)) */
            [19] = BSP_PRV_VECT_ENUM(EVENT_ADC12_FIFOOVF,GROUP3), /* ADC FIFOOVF (FIFO data overflow) */
            [20] = BSP_PRV_VECT_ENUM(EVENT_ADC12_FIFOREQ0,GROUP4), /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
            [21] = BSP_PRV_VECT_ENUM(EVENT_ADC12_FIFOREQ1,GROUP5), /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
            [22] = BSP_PRV_VECT_ENUM(EVENT_ADC12_FIFOREQ2,GROUP6), /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
            [23] = BSP_PRV_VECT_ENUM(EVENT_ADC12_FIFOREQ3,GROUP7), /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
            [24] = BSP_PRV_VECT_ENUM(EVENT_ADC12_FIFOREQ4,GROUP0), /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
            [25] = BSP_PRV_VECT_ENUM(EVENT_ADC12_FIFOREQ5678,GROUP1), /* ADC FIFOREQ5678 (FIFO data read request interrupt(Gr.5678)) */
            [26] = BSP_PRV_VECT_ENUM(EVENT_GPT4_CAPTURE_COMPARE_B,GROUP2), /* GPT4 CAPTURE COMPARE B (Capture/Compare match B) */
            [27] = BSP_PRV_VECT_ENUM(EVENT_GPT5_CAPTURE_COMPARE_A,GROUP3), /* GPT5 CAPTURE COMPARE A (Capture/Compare match A) */
            [28] = BSP_PRV_VECT_ENUM(EVENT_GPT5_CAPTURE_COMPARE_B,GROUP4), /* GPT5 CAPTURE COMPARE B (Capture/Compare match B) */
        };
        #endif
        #endif