/*
  LED Pong!

  Play the classic game Pong with an 64x32 RGB LED Matrix, controlled over WiFi by Blynk app. Reddit post for this project: https://www.reddit.com/r/arduino/comments/8kvg5o/led_pong/

  For hardware connection, check out the YouTube channel and Github of Brian Lough, aka. witnessmenow on Github, Reddit, Twitter and YouTube. Particularly this video: https://www.youtube.com/watch?v=YvU_ZfF7vs4

  For Blynk, create an account and a project. You will then need to add two sliders to control each racket. Set one slider to V13 pin and the other to V14. Value range should be set to "from 25 to 1". If you change the racket length in the code below, value range needs to change accordingly as well (but changing the racket length will cause the game to run haphazardly, as explained below). "Send values on release only" option must be off.

  Next, add two "value display" OR "labeled value" boxes for score display. The latter can be formatted to appear on the center but costs more Blynk energy (each user gets 2000 free energy, and this entire pong project costs 800 or 1200 energy, depending on your box choices). The score box belonging to the slider that controls V13 racket must be set to V15 pin, and the other to V16 pin. Label field must be "/pin/" without the quotation marks. Refresh interval must be push.

  Ready to run the display and the app. Upload the sketch, run the Blynk project (as of now, done via a triangle button on top right), and when the game begins, you should be able to control the rackets.

  Enjoy!

  P.S. As you'll no doubt surmise from my code, I'm a noob. Let me know if you have improvements, suggestions or advice (Github handle: uygarpolat). Thank you.
*/

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#define BLYNK_PRINT Serial
#include <Ticker.h>
#include <PxMatrix.h>
#include <SimpleTimer.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "BLYNK_AUTH_TOKEN";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SSID";
char pass[] = "PASSWORD";

Ticker display_ticker;
SimpleTimer timer;

// Pins for LED MATRIX
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_OE 2
#define P_D 12
#define P_E 0

// PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
// PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

uint16 myCOLORS[8] = {myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK};

// ISR for display refresh
void display_updater()
{

  display.display(70);

}

int imageHeight = 32;
int imageWidth = 64;
int x = 0;
int y = 1;
int OriginalPingPongRacketValue = 1;
int PingPongRacketValue = 1;

// Length of the rackets. The code is written with
// length 6 in mind, so it would be a hassle to
// change it (e.g. the ball bounces with different
// angles, depending on which of the
// 6 possible positions it hits the racket. If you
// change the racket length, you need to come up with
// the corresponding angles as well.
int PingPongRacketLength = 6;
int PingPongRacketLength2 = 6;

int OriginalPingPongRacketValue2 = 1;
int PingPongRacketValue2 = 1;
int BallX = 32; // Initial x-coordinate of the ball, mid-screen
int BallY = random(2, 30); // Initial y-coordinate of the ball, a random pixel.
int IncrementX = 1;
int IncrementY = 1;
int BlueScore = 0;
int GreenScore = 0;
int BallXreplacement;
int BallYreplacement;
int counterReplacement = 0;
int initiator = 0;
int initiator2 = 0;
int counterReplacementX = 0;
int counterReplacementY = 0;


// y-coordinate of the top of the right rocket.
// The way I wrote the code, the range of V13 and V14
// needs to be determined within the app. For instance,
// Ranges for both V13 and V14 must be 25 to 1. "Send values on
// release only" option must be off.
BLYNK_WRITE(V13) {
  PingPongRacketValue = param.asInt();
}

// y-coordinate of the top of the left rocket.
BLYNK_WRITE(V14) {
  PingPongRacketValue2 = param.asInt();
}

void setup() {

  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);

  display.begin(16);
  display.clearDisplay();
  display_ticker.attach(0.002, display_updater);
  yield();
  display.clearDisplay();

  Blynk.syncVirtual(V13); // Gets the position of the left slider from the app.
  Blynk.syncVirtual(V14); // Gets the position of the right slider from the app.
  Blynk.virtualWrite(V15, 0); // Displays, in the app, zero as the score of the left player.
  Blynk.virtualWrite(V16, 0); // Displays, in the app, zero as the score of the right player.

  // Displays introduction.
  displayName("LED", "PONG!");
  delay(3000);
  display.clearDisplay();

  // Paints borders.
  PaintEdges();

  // Counts down from 5 to start the game.
  for (int b = 5; b > 0; b--) {
    display.setTextSize(2);
    display.setTextColor(myCYAN);
    display.setCursor(27, 9);
    display.print(b);
    delay(1000);
    clearArea(27, 9, 36, 22);
  }

  // Draws the racket of left player.
  for (int i = PingPongRacketValue; i < PingPongRacketValue + PingPongRacketLength; i++)
    display.drawPixel(2, i, myBLUE);

  // Draws the rocket of right player.
  for (int i = PingPongRacketValue2; i < PingPongRacketValue2 + PingPongRacketLength2; i++)
    display.drawPixel(61, i, myGREEN);

  // Displays the current score (0-0).
  displayScore();

  // Running three functions: two for controlling the rackets, one for the ball. Changing the "moveBall" frequency changes the speed of the ball.
  timer.setInterval(1, movePingPongRacket);
  timer.setInterval(1, movePingPongRacket2);
  timer.setInterval(35, moveBall);
}

void loop() {
  Blynk.run(); 
  timer.run();
}

// Displays the racket of left player, runs every 1 millisecond.
void movePingPongRacket () {
  if (PingPongRacketValue != OriginalPingPongRacketValue) {
    for (int i = 1; i < 31; i++)
      display.drawPixel(2, i, myBLACK);
    for (int i = 0; i < PingPongRacketLength; i++)
      display.drawPixel(2, PingPongRacketValue + i, myBLUE);
    OriginalPingPongRacketValue = PingPongRacketValue;
  }
}

// Displays the racket of right player, runs every 1 millisecond.
void movePingPongRacket2 () {
  if (PingPongRacketValue2 != OriginalPingPongRacketValue2) {
    for (int i = 1; i < 31; i++)
      display.drawPixel(61, i, myBLACK);
    for (int i = 0; i < PingPongRacketLength2; i++)
      display.drawPixel(61, PingPongRacketValue2 + i, myGREEN);
    OriginalPingPongRacketValue2 = PingPongRacketValue2;
  }
}

// Moves ball around, runs every 35 milliseconds.
void moveBall () {
  // Clears the previous position of the ball.
  display.drawPixel (BallX, BallY, myBLACK);

  if (counterReplacementY == 1) {
    display.drawPixel(BallX, BallYreplacement, myBLACK);
  }
  if (counterReplacementX == 1) {
    display.drawPixel(BallXreplacement, BallY , myBLACK);
  }
  counterReplacementY = 0;
  counterReplacementX = 0;

  // Displays the current position of the ball.
  BallX = BallX + IncrementX;
  BallY = BallY + IncrementY;
  display.drawPixel(BallX, BallY, myCYAN);

  // Clears the area where the score is displayed, regardless of the score has just been displayed or not. This can be optimized to run only after the score is displayed.
  if (((BallX == 24 || BallX == 25) && IncrementX < 0) || ((BallX == 40 || BallX == 39) && IncrementX > 0)) {
    clearArea(10, 9, 19, 22);
    clearArea(43, 9, 52, 22);
  }

  // What to do when the ball hits, or fails to hit, the rackets.
  if (BallX % (imageWidth - 7) == 3) {
    int k = 0;
    if (BallX < imageWidth / 2) { //If player on the left is supposed to hit the ball...
      for (int i = 0; i < PingPongRacketLength; i++) {
        if (BallY == PingPongRacketValue + i) { //...see if the y-coordinate of the ball matches the y-coordinate of the racket.
          k++; // if, after the "for" loop, k returns as zero, a goal has been scored.
          if (i <= PingPongRacketLength / 2 - 1) { // If the ball hit the racket on the upper half of the racket, adjust increment (i.e. angle of the ball).
            if (i == 0) {
              IncrementX = 1;
              IncrementY = -2;
            }
            if (i == 1) {
              IncrementX = 1;
              IncrementY = -1;
            }
            if (i == 2) {
              IncrementX = 2;
              IncrementY = -1;
            }
          } else { // If the ball hit the racket on the lower half of the racket, adjust increment (i.e. angle of the ball).
            if (i == 3) {
              IncrementX = 2;
              IncrementY = 1;
            }
            if (i == 4) {
              IncrementX = 1;
              IncrementY = 1;
            }
            if (i == 5) {
              IncrementX = 1;
              IncrementY = 2;
            }
          }
        }
      }
    } else { // If player on the right is supposed to hit the ball... (similar process as above)
      for (int i = 0; i < PingPongRacketLength; i++) {
        if (BallY == PingPongRacketValue2 + i) {
          k++;
          if (i <= PingPongRacketLength2 / 2 - 1) {
            if (i == 0) {
              IncrementX = -1;
              IncrementY = -2;
            }
            if (i == 1) {
              IncrementX = -1;
              IncrementY = -1;
            }
            if (i == 2) {
              IncrementX = -2;
              IncrementY = -1;
            }
          } else {
            if (i == 3) {
              IncrementX = -2;
              IncrementY = 1;
            }
            if (i == 4) {
              IncrementX = -1;
              IncrementY = 1;
            }
            if (i == 5) {
              IncrementX = -1;
              IncrementY = 2;
            }
          }
        }
      }
    }
    if (k == 0) { // If the racket didn't hit the ball (i.e. a goal has been scored)...
      display.drawPixel (BallX, BallY, myBLACK); // Remove the ball from display.
      // Adjust the increments to 1 or -1, facing towards whoever just scored the goal.
      // This step ensures (1) ease of play for goal receiver (i.e. they don't keep
      // receiving the ball after they just allowed a goal) and (2) angles other than
      // 45 degrees (that is, othar than (1,1), (-1,-1), (1,-1), (-1,1)) are difficult
      // to catch when they originate from mid-screen, which they will, now that a
      // goal has been scored.
      if (IncrementX % 2 == 0)
        IncrementX = IncrementX / 2;
      if (IncrementY % 2 == 0)
        IncrementY = IncrementY / 2;
      IncrementX = IncrementX * -1;
      IncrementY = IncrementY * -1;
      if (BallX > imageWidth / 2) { // If the goal has been scored on the right side...
        BlueScore++; // Increase left player's score by one.
        Blynk.virtualWrite(V15, BlueScore); // Display the new score for the left player on the app.
        if (BlueScore == 10) { // If 10 is reached, display the winner, update the score on the app, and end the game.
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(myBLUE);
          display.setCursor(1, 1);
          display.print("BLUE");
          display.setCursor(1, 17);
          display.print("WINS!");
          delay(5000);
          Blynk.virtualWrite(V15, 0);
          Blynk.virtualWrite(V16, 0);
          delay(1000000); // In effect, stops the game. There is almost certainly a better way to do this.

        }
      } else {
        GreenScore++;
        Blynk.virtualWrite(V16, GreenScore);
        if (GreenScore == 10) {
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(myGREEN);
          display.setCursor(1, 1);
          display.print("GREEN");
          display.setCursor(1, 17);
          display.print("WINS!");
          delay(5000);
          Blynk.virtualWrite(V15, 0);
          Blynk.virtualWrite(V16, 0);
          delay(1000000);

        }
      }
      displayScore(); // Display the score in the mid-screen.
      BallX = 32; // Re-adjust ball coordinates for the next point.
      BallY = random(2, 30);
    }
  }

  if (BallY % (imageHeight - 3) == 1) { //When the ball hits the up and down borders, change the direction.
    IncrementY = IncrementY * -1;
  }

  // The code was originally written for increments of only 1 or - 1.
  // After seeing that that makes the game too predictable, I introduced
  // increments of 2 and - 2, which did solve the predictability problem,
  // but introduced the problem of ball running outside the boundaries of
  // the field. When the increments were 1 and - 1, the ball would definitely
  // have to hit the line right before the boundaries (or the line
  // right before the rackets), and I could change the
  // direction according to when that happened. But with the introduction of
  // increments of 2 an - 2, the ball sometimes skipped the line right before
  // the boundary, and consequently didn't get caught by the direction changing
  // conditionals. The following two "if"s are solving this problem.
  if ((IncrementY == 2 && BallY == 29) || (IncrementY == -2 && BallY == 2)) { //When IncrementY is 2 or -2, and the ball will go outside of the borders in the next iteration, adjust the ball so that it doesn't go out of bounds via up or down.
    BallYreplacement = BallY;
    BallY = BallY - IncrementY / 2;
    counterReplacementY = 1;
    if (initiator == 0) {
      BallXreplacement = BallX;
      initiator = 1;
    }
  }

  if ((IncrementX == 2 && BallX == 59) || (IncrementX == -2 && BallX == 4)) { //When IncrementX is 2 or -2, and the ball will go outside of the rackets in the next iteration, adjust the ball so that it doesn't go out of bounds via left or right.
    BallXreplacement = BallX;
    BallX = BallX - IncrementX / 2;
    counterReplacementX = 1;
    if (initiator2 == 0) {
      BallYreplacement = BallY;
      initiator2 = 1;
    }
  }
}

// Paints borders.
void PaintEdges () {
  int WL = 1;
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 64; x++) {
      if (x % (64 - WL) < WL || y % (32 - WL) < WL)
        display.drawPixel(x , y, myCYAN);
    }
  }
}

// Displays score.
void displayScore() {
  display.setTextSize(2);
  display.setCursor(10, 9);
  display.print(BlueScore);
  display.setCursor(43, 9);
  display.print(GreenScore);
}

// Paints a specified area black. Format: clearArea (x-coordinate of top left, y-coordinate of top left, x-coordinate of bottom right, y-coordinate of bottom right)
void clearArea (int CleanXstart, int CleanYstart, int CleanXend, int CleanYend) {
  for (int k = CleanYstart; k <= CleanYend; k++) {
    for (int i = CleanXstart; i <= CleanXend; i++) {
      display.drawPixel(i, k, myBLACK);
    }
  }
}

// Displays a text. Original coder: Brian Lough, aka. witnessmenow on Github, Reddit, Twitter and YouTube.
void displayText(String text, int yPos) {
  int16_t  x1, y1;
  uint16_t w, h;
  display.setTextSize(2);
  char charBuf[text.length() + 1];
  text.toCharArray(charBuf, text.length() + 1);
  display.getTextBounds(charBuf, 0, yPos, &x1, &y1, &w, &h);
  int startingX = 33 - (w / 2);
  if (startingX < 0) {
    display.setTextSize(1);
    display.getTextBounds(charBuf, 0, yPos, &x1, &y1, &w, &h);
    startingX = 33 - (w / 2);
  }
  display.setCursor(startingX, yPos);
  Serial.println(startingX);
  Serial.println(yPos);
  display.print(text);
}

// Displays a text. Original coder: Brian Lough, aka. witnessmenow on Github, Reddit, Twitter and YouTube.
void displayName(String first, String last)
{
  display.setTextColor(myCYAN);
  displayText(first, 1);
  display.setTextColor(myCYAN);
  displayText(last, 17);
}
