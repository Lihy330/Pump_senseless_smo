/* generated vector header file - do not edit */
        #ifndef VECTOR_DATA_H
        #define VECTOR_DATA_H
        #ifdef __cplusplus
        extern "C" {
        #endif
                /* Number of interrupts allocated */
        #ifndef VECTOR_DATA_IRQ_COUNT
        #define VECTOR_DATA_IRQ_COUNT    (29)
        #endif
        /* ISR prototypes */
        void sci_b_uart_rxi_isr(void);
        void sci_b_uart_txi_isr(void);
        void sci_b_uart_tei_isr(void);
        void sci_b_uart_eri_isr(void);
        void comp_hs_int_isr(void);
        void gpt_capture_compare_a_isr(void);
        void adc_b_limclpi_isr(void);
        void adc_b_err0_isr(void);
        void adc_b_err1_isr(void);
        void adc_b_resovf0_isr(void);
        void adc_b_resovf1_isr(void);
        void adc_b_calend0_isr(void);
        void adc_b_calend1_isr(void);
        void adc_b_adi0_isr(void);
        void adc_b_adi1_isr(void);
        void adc_b_adi2_isr(void);
        void adc_b_adi3_isr(void);
        void adc_b_adi4_isr(void);
        void adc_b_adi5678_isr(void);
        void adc_b_fifoovf_isr(void);
        void adc_b_fiforeq0_isr(void);
        void adc_b_fiforeq1_isr(void);
        void adc_b_fiforeq2_isr(void);
        void adc_b_fiforeq3_isr(void);
        void adc_b_fiforeq4_isr(void);
        void adc_b_fiforeq5678_isr(void);
        void gpt_capture_compare_b_isr(void);

        /* Vector table allocations */
        #define VECTOR_NUMBER_SCI0_RXI ((IRQn_Type) 0) /* SCI0 RXI (Receive data full) */
        #define SCI0_RXI_IRQn          ((IRQn_Type) 0) /* SCI0 RXI (Receive data full) */
        #define VECTOR_NUMBER_SCI0_TXI ((IRQn_Type) 1) /* SCI0 TXI (Transmit data empty) */
        #define SCI0_TXI_IRQn          ((IRQn_Type) 1) /* SCI0 TXI (Transmit data empty) */
        #define VECTOR_NUMBER_SCI0_TEI ((IRQn_Type) 2) /* SCI0 TEI (Transmit end) */
        #define SCI0_TEI_IRQn          ((IRQn_Type) 2) /* SCI0 TEI (Transmit end) */
        #define VECTOR_NUMBER_SCI0_ERI ((IRQn_Type) 3) /* SCI0 ERI (Receive error) */
        #define SCI0_ERI_IRQn          ((IRQn_Type) 3) /* SCI0 ERI (Receive error) */
        #define VECTOR_NUMBER_ACMPHS0_INT ((IRQn_Type) 4) /* ACMPHS0 INT (High Speed Comparator channel 0 interrupt) */
        #define ACMPHS0_INT_IRQn          ((IRQn_Type) 4) /* ACMPHS0 INT (High Speed Comparator channel 0 interrupt) */
        #define VECTOR_NUMBER_GPT0_CAPTURE_COMPARE_A ((IRQn_Type) 5) /* GPT0 CAPTURE COMPARE A (Capture/Compare match A) */
        #define GPT0_CAPTURE_COMPARE_A_IRQn          ((IRQn_Type) 5) /* GPT0 CAPTURE COMPARE A (Capture/Compare match A) */
        #define VECTOR_NUMBER_ADC12_LIMCLPI ((IRQn_Type) 6) /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
        #define ADC12_LIMCLPI_IRQn          ((IRQn_Type) 6) /* ADC LIMCLPI (Limiter clip interrupt with the limit table 0 to 7) */
        #define VECTOR_NUMBER_ADC12_ERR0 ((IRQn_Type) 7) /* ADC ERR0 (A/D converter unit 0 Error) */
        #define ADC12_ERR0_IRQn          ((IRQn_Type) 7) /* ADC ERR0 (A/D converter unit 0 Error) */
        #define VECTOR_NUMBER_ADC12_ERR1 ((IRQn_Type) 8) /* ADC ERR1 (A/D converter unit 1 Error) */
        #define ADC12_ERR1_IRQn          ((IRQn_Type) 8) /* ADC ERR1 (A/D converter unit 1 Error) */
        #define VECTOR_NUMBER_ADC12_RESOVF0 ((IRQn_Type) 9) /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
        #define ADC12_RESOVF0_IRQn          ((IRQn_Type) 9) /* ADC RESOVF0 (A/D conversion overflow on A/D converter unit 0) */
        #define VECTOR_NUMBER_ADC12_RESOVF1 ((IRQn_Type) 10) /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
        #define ADC12_RESOVF1_IRQn          ((IRQn_Type) 10) /* ADC RESOVF1 (A/D conversion overflow on A/D converter unit 1) */
        #define VECTOR_NUMBER_ADC12_CALEND0 ((IRQn_Type) 11) /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
        #define ADC12_CALEND0_IRQn          ((IRQn_Type) 11) /* ADC CALEND0 (End of calibration of A/D converter unit 0) */
        #define VECTOR_NUMBER_ADC12_CALEND1 ((IRQn_Type) 12) /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
        #define ADC12_CALEND1_IRQn          ((IRQn_Type) 12) /* ADC CALEND1 (End of calibration of A/D converter unit 1) */
        #define VECTOR_NUMBER_ADC12_ADI0 ((IRQn_Type) 13) /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
        #define ADC12_ADI0_IRQn          ((IRQn_Type) 13) /* ADC ADI0 (End of A/D scanning operation(Gr.0)) */
        #define VECTOR_NUMBER_ADC12_ADI1 ((IRQn_Type) 14) /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
        #define ADC12_ADI1_IRQn          ((IRQn_Type) 14) /* ADC ADI1 (End of A/D scanning operation(Gr.1)) */
        #define VECTOR_NUMBER_ADC12_ADI2 ((IRQn_Type) 15) /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
        #define ADC12_ADI2_IRQn          ((IRQn_Type) 15) /* ADC ADI2 (End of A/D scanning operation(Gr.2)) */
        #define VECTOR_NUMBER_ADC12_ADI3 ((IRQn_Type) 16) /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
        #define ADC12_ADI3_IRQn          ((IRQn_Type) 16) /* ADC ADI3 (End of A/D scanning operation(Gr.3)) */
        #define VECTOR_NUMBER_ADC12_ADI4 ((IRQn_Type) 17) /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
        #define ADC12_ADI4_IRQn          ((IRQn_Type) 17) /* ADC ADI4 (End of A/D scanning operation(Gr.4)) */
        #define VECTOR_NUMBER_ADC12_ADI5678 ((IRQn_Type) 18) /* ADC ADI5678 (End of A/D scanning operation(Gr.5678)) */
        #define ADC12_ADI5678_IRQn          ((IRQn_Type) 18) /* ADC ADI5678 (End of A/D scanning operation(Gr.5678)) */
        #define VECTOR_NUMBER_ADC12_FIFOOVF ((IRQn_Type) 19) /* ADC FIFOOVF (FIFO data overflow) */
        #define ADC12_FIFOOVF_IRQn          ((IRQn_Type) 19) /* ADC FIFOOVF (FIFO data overflow) */
        #define VECTOR_NUMBER_ADC12_FIFOREQ0 ((IRQn_Type) 20) /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
        #define ADC12_FIFOREQ0_IRQn          ((IRQn_Type) 20) /* ADC FIFOREQ0 (FIFO data read request interrupt(Gr.0)) */
        #define VECTOR_NUMBER_ADC12_FIFOREQ1 ((IRQn_Type) 21) /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
        #define ADC12_FIFOREQ1_IRQn          ((IRQn_Type) 21) /* ADC FIFOREQ1 (FIFO data read request interrupt(Gr.1)) */
        #define VECTOR_NUMBER_ADC12_FIFOREQ2 ((IRQn_Type) 22) /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
        #define ADC12_FIFOREQ2_IRQn          ((IRQn_Type) 22) /* ADC FIFOREQ2 (FIFO data read request interrupt(Gr.2)) */
        #define VECTOR_NUMBER_ADC12_FIFOREQ3 ((IRQn_Type) 23) /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
        #define ADC12_FIFOREQ3_IRQn          ((IRQn_Type) 23) /* ADC FIFOREQ3 (FIFO data read request interrupt(Gr.3)) */
        #define VECTOR_NUMBER_ADC12_FIFOREQ4 ((IRQn_Type) 24) /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
        #define ADC12_FIFOREQ4_IRQn          ((IRQn_Type) 24) /* ADC FIFOREQ4 (FIFO data read request interrupt(Gr.4)) */
        #define VECTOR_NUMBER_ADC12_FIFOREQ5678 ((IRQn_Type) 25) /* ADC FIFOREQ5678 (FIFO data read request interrupt(Gr.5678)) */
        #define ADC12_FIFOREQ5678_IRQn          ((IRQn_Type) 25) /* ADC FIFOREQ5678 (FIFO data read request interrupt(Gr.5678)) */
        #define VECTOR_NUMBER_GPT4_CAPTURE_COMPARE_B ((IRQn_Type) 26) /* GPT4 CAPTURE COMPARE B (Capture/Compare match B) */
        #define GPT4_CAPTURE_COMPARE_B_IRQn          ((IRQn_Type) 26) /* GPT4 CAPTURE COMPARE B (Capture/Compare match B) */
        #define VECTOR_NUMBER_GPT5_CAPTURE_COMPARE_A ((IRQn_Type) 27) /* GPT5 CAPTURE COMPARE A (Capture/Compare match A) */
        #define GPT5_CAPTURE_COMPARE_A_IRQn          ((IRQn_Type) 27) /* GPT5 CAPTURE COMPARE A (Capture/Compare match A) */
        #define VECTOR_NUMBER_GPT5_CAPTURE_COMPARE_B ((IRQn_Type) 28) /* GPT5 CAPTURE COMPARE B (Capture/Compare match B) */
        #define GPT5_CAPTURE_COMPARE_B_IRQn          ((IRQn_Type) 28) /* GPT5 CAPTURE COMPARE B (Capture/Compare match B) */
        #ifdef __cplusplus
        }
        #endif
        #endif /* VECTOR_DATA_H */