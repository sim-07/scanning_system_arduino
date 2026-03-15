#include <Arduino.h>

const int STEPPER_PINS[] = {4, 5, 6, 7};
const int SERVO_PIN = 9;
const int TRIG_PIN = 2;
const int ECHO_PIN = 3;

const int PWM_MAX = 4000;
const int PWM_MIN = 2000;
const int FRAME_TOT = (PWM_MAX - PWM_MIN) / 10;
const float PWM_DEG = 0.045; // gradi per PWM

void scan(int);
void aim(int);

bool scan_completed = false;
double ambient_map[FRAME_TOT + 1];
int frame_n = 0;
int position = 2100;
int frame_tot = (PWM_MAX - PWM_MIN) / 10;
int delay_t = 30;
int frame_target_destroyed = -1;

void setup()
{
    for (int i = 0; i < 4; i++)
        pinMode(STEPPER_PINS[i], OUTPUT);

    pinMode(SERVO_PIN, OUTPUT);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    TCCR1A = (1 << COM1A1) | (1 << WGM11);              // Fast PWM, 16-bit
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Prescaler 8
    ICR1 = 39999;                                       // frequenza a 50Hz

    Serial.begin(9600);
}

void loop()
{

    // 1 grado = 22.2 PWM
    OCR1A = PWM_MIN;
    delay(500);

    frame_n = 0;
    for (int i = PWM_MIN; i < PWM_MAX; i += 10)
    {
        if (frame_n < 0)
            frame_n = 0;

        if (frame_n > FRAME_TOT)
            frame_n = FRAME_TOT;

        OCR1A = i;
        position = i;
        delay(delay_t);

        scan(frame_n);
        frame_n++;
    }

    frame_n = FRAME_TOT;

    for (int i = PWM_MAX; i >= PWM_MIN; i -= 10)
    {
        if (frame_n < 0)
            frame_n = 0;

        if (frame_n > FRAME_TOT)
            frame_n = FRAME_TOT;

        OCR1A = i;
        position = i;
        delay(delay_t);

        scan(frame_n);
        frame_n--;
    }

    scan_completed = true;
}

void scan(int frame_n)
{

    double duration;
    double distance;

    if (scan_completed)
    {
        delay_t = 20;
    }
    else
    {
        delay_t = 30;
    }

    int v_scan = 0;
    long sum_d = 0;

    for (int i = 0; i < 3; i++)
    {
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);

        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        long d = pulseIn(ECHO_PIN, HIGH, 30000);
        float dis = d * 0.034 / 2;

        if (dis > 30 && dis < 400)
        {
            v_scan++;
            sum_d += d;
        }

        delay(5);
    }

    if (v_scan == 0)
    {
        Serial.println("Error Out of Range");
        return;
    }

    duration = sum_d / v_scan;

    if (duration == 0)
    {
        Serial.println("Out of range");
        return;
    }
    else
    {
        distance = duration * 0.034 / 2;

        if (!scan_completed)
        {
            ambient_map[frame_n] = distance;
        }
        else
        {
            if (distance > 400)
                return;

            if ((ambient_map[frame_n] - distance > 30) && (ambient_map[frame_n] - distance < 300) && abs(frame_n - frame_target_destroyed) > 15)
            {
                Serial.print("POSSIBILE BERSAGLIO al grado: ");
                Serial.println(frame_n);

                int l_min = position - 400;
                int l_max = position + 400;

                if (l_min < PWM_MIN)
                    l_min = PWM_MIN;

                if (l_max > PWM_MAX)
                    l_max = PWM_MAX;

                for (int i = l_min; i < l_max; i += 10)
                {

                    OCR1A = i;
                    delay(30);

                    int curr_frame = (i - PWM_MIN) / 10;

                    if (curr_frame < 0)
                        curr_frame = 0;

                    if (curr_frame > FRAME_TOT)
                        curr_frame = FRAME_TOT;

                    position = i;

                    digitalWrite(TRIG_PIN, LOW);
                    delayMicroseconds(2);

                    digitalWrite(TRIG_PIN, HIGH);
                    delayMicroseconds(10);
                    digitalWrite(TRIG_PIN, LOW);

                    duration = pulseIn(ECHO_PIN, HIGH, 30000);

                    float distance_t = duration * 0.034 / 2;

                    if (distance_t > 0 && (ambient_map[curr_frame] - distance_t > 20))
                    {
                        Serial.print("BERSAGLIO al grado: ");
                        Serial.println(curr_frame);

                        frame_target_destroyed = frame_n;

                        aim(curr_frame);

                        break;
                    }
                }
            }
            else
            {
                Serial.print("Nessun movimento. Distanza: ");
                Serial.print(distance);
                Serial.println(" cm");
            }
        }
    }
}

void aim(int frame_n)
{

    float deg = ((frame_n * 10) * PWM_DEG) - (PWM_DEG * ((PWM_MAX - PWM_MIN) / 2)); // lo stepper mira inizialmente dritto, mentre il servo gira da -45 a +45
    int steps = abs(deg) * 1.415;                                                   // 1.415 giri per ogni grado.

    const int sequenza[4][4] = {
        {HIGH, LOW, LOW, LOW},
        {LOW, HIGH, LOW, LOW},
        {LOW, LOW, HIGH, LOW},
        {LOW, LOW, LOW, HIGH}};

    for (int p = 0; p < steps; p++)
    {
        if (deg < 0)
        {
            // Senso orario
            for (int fase = 0; fase < 4; fase++)
            {
                for (int i = 0; i < 4; i++)
                    digitalWrite(STEPPER_PINS[i], sequenza[fase][i]);
                delay(10);
            }
        }
        else
        {
            // Senso antiorario
            for (int fase = 3; fase >= 0; fase--)
            {
                for (int i = 0; i < 4; i++)
                    digitalWrite(STEPPER_PINS[i], sequenza[fase][i]);
                delay(10);
            }
        }
    }

    // TODO SHOOT

    for (int i = 0; i < 4; i++)
        digitalWrite(STEPPER_PINS[i], LOW);

    // ritorno a 0
    delay(5000);

    for (int p = 0; p < steps; p++)
    {
        if (deg < 0)
        {
            for (int fase = 3; fase >= 0; fase--)
            {
                for (int i = 0; i < 4; i++)
                    digitalWrite(STEPPER_PINS[i], sequenza[fase][i]);
                delay(10);
            }
        }
        else
        {
            for (int fase = 0; fase < 4; fase++)
            {
                for (int i = 0; i < 4; i++)
                    digitalWrite(STEPPER_PINS[i], sequenza[fase][i]);
                delay(10);
            }
        }
    }

    for (int i = 0; i < 4; i++)
        digitalWrite(STEPPER_PINS[i], LOW);
}
