/*
 Name:		NanoFurnace.ino
 Created:	08.02.2019 17:45:25
 Author:	Ksiw
*/
//#include <avr/wdt.h>
//---------------------------------------------------------------
#define START_ALL PORTD7  
#define FAN PORTD2
#define SPARK PORTD3
#define VALVE PORTD4
#define TEMPER_SENSOR PORTC0
#define TEMPERATURE_UP 200     //разница роста температуры при включении, примерно 20 град
#define TEMPERATURE_WORK 520   //~температура работы не ниже примерно 50град
#define DEBUG true			//закомментировать для релиза
#define FAN_COLD   (3000*60)    // охлаждение после остановки
//---------------------------------------------------------------
#define FAN_DELAY 2000
#define SPARK_DELAY 2000 //10000
#define VALVE_DELAY 3000 //30000
//---------------------------------------------------------------
bool startAll, fanOn, sparkOn, valveToggle; 
bool temperUp;
uint8_t analog_ref = DEFAULT;
uint16_t temperTemp;
volatile uint32_t result;

//---------------------------------------------------------------
boolean StartAll();
void FanStart(bool &);
void SparkToggle(bool &, bool &);
void VavleToggle(bool &, bool &, bool &, bool&);
void TemperUp(bool &);
void StopAll(bool &, bool &, bool &, bool&);
void ADC_init();
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void setup()
{
	fanOn = sparkOn = valveToggle = temperUp = false;
	DDRD |= 1 << FAN;
	DDRD |= 1 << SPARK;
	DDRD |= 1 << VALVE;
	PORTD &= ~(1 << START_ALL);
	ADC_init();
	//wdt_enable(WDTO_4S);
#ifdef DEBUG //++++++++++++++++
	Serial.begin(115200)
#endif // DEBUG---------------

	;

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void loop() 
{
	while (true)
	{
		while (StartAll())
		{
			FanStart(fanOn);
			temperTemp = result;
			SparkToggle(fanOn, sparkOn);
			
			#ifdef DEBUG //++++++++++++++++++++
				Serial.print("base ");
				Serial.println(result);
			#endif // DEBUG---------------------
			
			VavleToggle(fanOn, sparkOn, valveToggle, temperUp);
			StopAll(fanOn, sparkOn, valveToggle, temperUp);
			while (StartAll())
			{
			
			#ifdef DEBUG //++++++++++++++++++++
				Serial.println("DELAY ");
				_delay_ms(500);
			#endif // DEBUG--------------------

			_delay_ms(1);
			}
			//wdt_reset();
		}
		
	#ifdef DEBUG //++++++++++++++++++++++
		_delay_ms(500);
		Serial.print("OFF ");
		Serial.println(result);
	#endif // DEBUG------------------------
	}
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
boolean StartAll()
{
	return bitRead(PIND, START_ALL);
}
//---------------------------------------------------------------------------
void FanStart(bool &fan)
{
		PORTD |= 1 << FAN;
		_delay_ms(FAN_DELAY);
		fan = true;
}
//---------------------------------------------------------------------------
void SparkToggle(bool &fan, bool &spark)
{
	if (fan && !spark)
	{	
		PORTD |= 1 << SPARK;
		spark = true;
	}
	else
	{
		PORTD &= ~(1 << SPARK);
	}
}
//---------------------------------------------------------------------------
void VavleToggle(bool &fan, bool &spark, bool &valve, bool &temperature)
{


	if (fan && spark && !valve)
	{
		_delay_ms(2000);
		PORTD |= 1 << VALVE;
		valve = true;
	}
	else
	{
		PORTD &= ~(1 << VALVE);
	}

	_delay_ms(SPARK_DELAY);
	SparkToggle(fanOn, sparkOn);
	TemperUp(temperature);
	
	if(temperUp)
		while (StartAll() && (result>TEMPERATURE_WORK))
		{
			#ifdef DEBUG //++++++++++++++			
				_delay_ms(100);
				Serial.print("work ");
				Serial.println(result);
			#endif // DEBUG---------------

			_delay_ms(1);
		}
}
//---------------------------------------------------------------------------
void TemperUp(bool &t)
{
	_delay_ms(VALVE_DELAY - SPARK_DELAY);
	if ((result - temperTemp) > TEMPERATURE_UP)
		t = true;
}
//---------------------------------------------------------------------------
void StopAll(bool &fan, bool &spark, bool &valve, bool &temperup)
{
    //PORTD &= ~(1 << SPARK);
	PORTD &= ~(1 << VALVE);
	_delay_ms(FAN_COLD);
	PORTD &= ~(1 << FAN);
	fan = spark = valve = temperup = false;

}
//---------------------------------------------------------------------------
void ADC_init()
{
	ADCSRA = 0;             
	ADCSRB = 0;             
	ADMUX |= (1 << REFS0);  
	ADMUX |= (1 << REFS1);
	ADMUX |= (TEMPER_SENSOR & 0x07);    //A0 
	ADCSRA |= 0b00000111;  //макс разрядность
	ADCSRA |= (1 << ADATE);
	ADCSRA |= (1 << ADIE);  
	ADCSRA |= (1 << ADEN);
	ADCSRA |= (1 << ADSC);
}
//---------------------------------------------------------------------------
ISR(ADC_vect)
{
	result = ADCL | (ADCH << 8);
}