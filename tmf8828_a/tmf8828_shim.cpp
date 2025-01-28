/*
 *****************************************************************************
 * Copyright by ams OSRAM AG                                                       *
 * All rights are reserved.                                                  *
 *                                                                           *
 * IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
 * THE SOFTWARE.                                                             *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
 *****************************************************************************
 */

#include "tmf8828_shim.h"
#include "tmf8828.h"


void delayInMicroseconds ( uint32_t wait )
{
    sleep_ms( wait );
}

uint32_t getSysTick ( )
{
  //return 0;
  return (uint32_t) get_absolute_time();
}

uint8_t readProgramMemoryByte ( const uint8_t * ptr )
{
  uint32_t address = (uint32_t)ptr;
  return *(uint8_t *)address;
  //return pgm_read_byte( address );
}

void enablePinHigh ( void * dptr )
{
  (void)dptr; // not used here
  gpio_put(ENABLE_PIN, true);
}

void enablePinLow ( void * dptr )
{
  (void)dptr; // not used here
  gpio_put(ENABLE_PIN, false);
}

void configurePins ( void * dptr )
{
  (void)dptr; // not used here
  // configure ENABLE pin and interupt pin
  gpio_init(ENABLE_PIN);
  gpio_set_dir(ENABLE_PIN, GPIO_OUT);
  gpio_init(INTERRUPT_PIN);
  gpio_set_dir(INTERRUPT_PIN, GPIO_IN);
  gpio_pull_up(INTERRUPT_PIN);
  gpio_init(TRIGGER_INTERRUPT_PIN);           // if interrupt PIN is used        
}


void i2cOpen ( void * dptr, uint32_t i2cClockSpeedInHz )
{
  (void)dptr; // not used here
  i2c_init(i2c0,i2cClockSpeedInHz);
  gpio_set_function(16, GPIO_FUNC_I2C);
  gpio_set_function(17, GPIO_FUNC_I2C);
  gpio_pull_up(16);
  gpio_pull_up(17);
}

void i2cClose ( void * dptr )
{
  (void)dptr; // not used here
  i2c_deinit(i2c0);
}


void printChar ( char c ) 
{
   printf("%c", c);
}

void printInt ( int32_t i )
{
    printf("%d", i);
}

void printUint ( uint32_t i )
{
    printf("%u", i);
}

void printUintHex ( uint32_t i )
{
    printf("%X", i);
}

void printStr ( char * str )
{
    printf("%s", str);
    // use only for printing zero-terminated strings: (const char *)
}

void printLn ( void )
{
    printf("\n");
}


// function prints a single result, and returns incremented pointer
static uint8_t * print_result ( tmf8828Driver * driver, uint8_t * data )
{
  uint8_t confidence = data[0];               // 1st byte is confidence
  uint16_t distance = data[2];                // 3rd byte is MSB distance
  distance = (distance << 8) + data[1];       // 2nd byte is LSB distnace
  distance = tmf8828CorrectDistance( driver, distance );
  PRINT_CHAR( SEPARATOR );
  PRINT_INT( distance );
  PRINT_CHAR( SEPARATOR );
  PRINT_INT( confidence );
  return data+3;                              // for convenience only, return the new pointer
}

// Results printing:
// #Obj,<i2c_slave_address>,<result_number>,<temperature>,<number_valid_results>,<systick>,<distance_0_mm>,<confidence_0>,<distance_1_mm>,<distance_1>, ...
void printResults ( void * dptr, uint8_t * data, uint8_t len )
{
  tmf8828Driver * driver = (tmf8828Driver *)dptr;  
  if ( len >= TMF8828_COM_CONFIG_RESULT__measurement_result_size )
  {
    int8_t i;
    uint32_t sysTick = tmf8828GetUint32( data + RESULT_REG( SYS_TICK_0 ) );
    PRINT_STR( "#Obj" );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( driver->i2cSlaveAddress );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( data[ RESULT_REG( RESULT_NUMBER) ] );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( data[ RESULT_REG( TEMPERATURE )] );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( data[ RESULT_REG( NUMBER_VALID_RESULTS )] );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( sysTick );
    data = data + RESULT_REG( RES_CONFIDENCE_0 );
    for ( i = 0; i < PRINT_NUMBER_RESULTS ; i++ )
    {
      data = print_result( driver, data );
    }
    PRINT_LN( );
  }
  else // result structure too short
  {
    PRINT_STR( "#Err" );
    PRINT_CHAR( SEPARATOR );
    PRINT_STR( "result too short" );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( len );
    PRINT_LN( );
  }
}

// Print histograms:
// #Raw,<i2c_slave_address>,<sub_packet_number>,<data_0>,<data_1>,..,,<data_127>
// #Cal,<i2c_slave_address>,<sub_packet_number>,<data_0>,<data_1>,..,,<data_127>
void printHistogram ( void * dptr, uint8_t * data, uint8_t len )
{
  tmf8828Driver * driver = (tmf8828Driver *)dptr;  
  if ( len >= TMF8828_COM_HISTOGRAM_PACKET_SIZE )
  {
    uint8_t i;
    uint8_t * ptr = &( data[ RESULT_REG( SUBPACKET_PAYLOAD_0 ) ] );
    if ( data[0] & TMF8828_COM_HIST_DUMP__histogram__raw_24_bit_histogram )
    { 
      PRINT_STR( "#Raw" );
    }
    else if ( data[0] & TMF8828_COM_HIST_DUMP__histogram__electrical_calibration_24_bit_histogram )
    {
      PRINT_STR( "#Cal" );
    }
    else 
    {
      PRINT_STR( "#???" );
    }
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( driver->i2cSlaveAddress );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( data[ RESULT_REG( SUBPACKET_NUMBER ) ] );          // print the sub-packet number indicating the third-of-a-channel/tdc the histogram belongs to  
    
    for ( i = 0; i < TMF8828_NUMBER_OF_BINS_PER_CHANNEL ; i++, ptr++ )
    {
      PRINT_CHAR( SEPARATOR );
      PRINT_INT( *ptr );
    }
    PRINT_LN( );
  }
  // else structure too short
}

void inputOpen ( uint32_t baudrate )
{
  // Serial.end( ); // this clears any old pending data 
  // Serial.begin( baudrate );
  // No direct equivalent in stdio, typically initialization is not needed
}

void inputClose ( )
{
  // Serial.end( );
  // No direct equivalent in stdio, typically cleanup is not needed
}

int8_t inputGetKey ( char *c )
{
  int ch = getchar_timeout_us(0); // Non-blocking read with 0 timeout
  if (ch == PICO_ERROR_TIMEOUT) {
      return 0;
  }
  *c = (char)ch;
  return 1;
}

void printConstStr ( const char * str )
{
  printf("%s", str);
}

void pinOutput ( uint8_t pin )
{
  // pinMode( pin, OUTPUT ); // define a pin as output
  // No direct equivalent in stdio, typically handled by GPIO library
}

void pinInput ( uint8_t pin )
{ 
  // pinMode( pin, INPUT ); // define a pin as input
  // No direct equivalent in stdio, typically handled by GPIO library
}

void setInterruptHandler( void (* handler)( void ) )
{
  // attachInterrupt( digitalPinToInterrupt( INTERRUPT_PIN ), handler, FALLING );
  // No direct equivalent in stdio, typically handled by GPIO library
}

void disableInterruptHandler( uint8_t pin )
{
  // detachInterrupt( digitalPinToInterrupt( pin ) );
  // No direct equivalent in stdio, typically handled by GPIO library
}

void disableInterrupts ( void )
{
  // noInterrupts( );
  // No direct equivalent in stdio, typically handled by system calls
}

void enableInterrupts ( void )
{
  // interrupts( );
  // No direct equivalent in stdio, typically handled by system calls
}

char inputGetKey ( )  
{
    int ch = getchar_timeout_us(0); // Non-blocking read with 0 timeout
    return (ch != PICO_ERROR_TIMEOUT) ? (char)ch : 0;
}


// ----------------------------------------- i2c ---------------------------------------

static int8_t i2cTxOnly ( uint8_t logLevel, uint8_t slaveAddr, uint8_t regAddr, uint16_t toTx, const uint8_t * txData )
{  // split long transfers into max of 32-bytes: 1 byte is register address, up to 31 are payload.
  int8_t res = I2C_SUCCESS;
  //printf("i2cTxOnly, slaveAddr: %x, regAddr: %x, toTx: %d, txData: %s\n", slaveAddr, regAddr, toTx, txData);
  do 
  {
    //printf("do start\n");
    uint8_t tx;
    if ( toTx > ARDUINO_MAX_I2C_TRANSFER - 1) 
    {
      tx = ARDUINO_MAX_I2C_TRANSFER - 1;
    }
    else 
    {
      tx = toTx; // less than 31 bytes 
    }
    if ( logLevel & TMF8828_LOG_LEVEL_I2C ) 
    {
      PRINT_STR( "I2C-TX (0x" );
      PRINT_UINT_HEX( slaveAddr );
      PRINT_STR( ")" );
      PRINT_STR( " tx=" );
      PRINT_INT( tx+1 );          // +1 for regAddr
      PRINT_STR( " 0x" );
      PRINT_UINT_HEX( regAddr );
      if ( logLevel >= TMF8828_LOG_LEVEL_DEBUG ) 
      {
        uint8_t dumpTx = tx;
        const uint8_t * dump = txData;
        while ( dumpTx-- )
        {
          PRINT_STR( " 0x" );
          PRINT_UINT_HEX( *dump );
          dump++;
        }
      }
      PRINT_LN( );
    }

    uint8_t buffer[ARDUINO_MAX_I2C_TRANSFER];
    buffer[0] = regAddr;
    memcpy(&buffer[1], txData, tx);
    //printf("i2c write start\n");
    res = i2c_write_blocking(i2c0, slaveAddr, buffer, tx + 1, false);
    if (res != PICO_ERROR_GENERIC)
    {
        res = I2C_SUCCESS;
    }
    //printf("i2c_write_blocking, slaveAddr: %x, buffer: %s, tx: %d, res: %d toTx: %d\n", slaveAddr, buffer, tx, res, toTx);
    toTx -= tx;
    txData += tx;
    regAddr += tx;
  } while ( toTx && res == I2C_SUCCESS );
  //printf("i2cTxOnly done res: %d\n", res);
  return res;
}

static int8_t i2cRxOnly ( uint8_t logLevel, uint8_t slaveAddr, uint16_t toRx, uint8_t * rxData )
{   // split long transfers into max of 32-bytes
  uint8_t expected = 0;
  uint8_t rx = 0;
  int8_t res = I2C_SUCCESS;
  //printf("i2cRxOnly, slaveAddr: %d, toRx: %d, rxData: %s\n", slaveAddr, toRx, rxData);
  do 
  {
    uint8_t * dump = rxData; // in case we dump on uart, we need the pointer
    if ( toRx > ARDUINO_MAX_I2C_TRANSFER ) 
    {
      expected = ARDUINO_MAX_I2C_TRANSFER;
    }
    else 
    {
      expected = toRx; // less than 32 bytes 
    }
    res = i2c_read_blocking(i2c0, slaveAddr, rxData, expected, false);
    //printf("rxdata: %s, expected: %d, rx: %d\n", *rxData, expected, rx);
    rx = expected;
    toRx -= expected;
    rxData += expected;
    if ( logLevel & TMF8828_LOG_LEVEL_I2C ) 
    {
      PRINT_STR( "I2C-RX (0x" );
      PRINT_UINT_HEX( slaveAddr );
      PRINT_STR( ")" );
      PRINT_STR( " toRx=" );
      PRINT_INT( rx );
      if ( logLevel >= TMF8828_LOG_LEVEL_DEBUG ) 
      {
        uint8_t dumpRx = rx;
        while ( dumpRx-- )
        {
          PRINT_STR( " 0x" );
          PRINT_UINT_HEX( *dump );
          dump++;
        }
      }
      PRINT_LN( );
    }
  } while ( toRx && expected == rx );
  if ( toRx || expected != rx )
  {
    res = I2C_ERR_TIMEOUT;
  }
  //printf("i2cRxOnly done res: %d\n", res);
  return res;
}

int8_t i2cTxReg ( void * dptr, uint8_t slaveAddr, uint8_t regAddr, uint16_t toTx, const uint8_t * txData )
{  // split long transfers into max of 32-bytes
  tmf8828Driver * driver = (tmf8828Driver *)dptr;
  return i2cTxOnly( driver->logLevel, slaveAddr, regAddr, toTx, txData ); 
}

int8_t i2cRxReg ( void * dptr, uint8_t slaveAddr, uint8_t regAddr, uint16_t toRx, uint8_t * rxData )
{   // split long transfers into max of 32-bytes
  tmf8828Driver * driver = (tmf8828Driver *)dptr;
  int8_t res = i2cTxOnly( driver->logLevel, slaveAddr, regAddr, 0, 0 ); 
  //printf( "Tx res %d\n", res );
  if ( res == I2C_SUCCESS )
  {
    res = i2cRxOnly( driver->logLevel, slaveAddr, toRx, rxData );
  }
  return res;
}

int8_t i2cTxRx ( void * dptr, uint8_t slaveAddr, uint16_t toTx, const uint8_t * txData, uint16_t toRx, uint8_t * rxData )
{
  tmf8828Driver * driver = (tmf8828Driver *)dptr;
  int8_t res = I2C_SUCCESS;
  if ( toTx )
  {
    res = i2cTxOnly( driver->logLevel, slaveAddr, *txData, toTx-1, txData+1 );
  }
  if ( toRx && res == I2C_SUCCESS )
  {
    res = i2cRxOnly( driver->logLevel, slaveAddr, toRx, rxData );
  }
  return res;
}
