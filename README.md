# Thermostat_2
Thermostat with programmability, weather, geofencing,, internaland external data logging, and web interface
This is working such as it is but is still very much a work in progress.
Originally desiigned for a Core, I ran out of resources soon,  The Photon in proving ample room anmd features for fully fleshing this out.

This is based on the original Spark project to make a thermostat similar to Nest.
Changes made to hardware include using an Ili9340 display and including weather information and display.
I utilize the built-in MicroSD card reader for logging of data locally and also logging it to a Google Sheet in Google Docs 

  -Weather is obtained from Weather Underground through webhooks
  -Sheet update is through Google Script Web App
  -Web app is HTML 5 with JQuery
  
