int uart_open(char*) ;
void uart_configure(int uart_stream_id) ;
int uart_tx(int uart_id, char* buffer, int size) ;
int uart_rx(int uart_id, char* buffer, int size) ;
void uart_close(int uart_stream_id) ;

