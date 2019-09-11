Use the sample, you can adapt screen.

1.prepare the screen drv.
   Make sure HI_HAL_SCREEN_Init is ready.

2.Modified the REF media_app_zip.bin.
   At the function PDT_INIT_MediaInit() in hi_product_init_service.c£¬
      return HI_SUCCESS at first line.
   Then make a new media_app_zip.bin.

3.In linux server, don't start main_app.
   You can change the bootapp in /app/ in borad.
   Just #./main_app&

4.Run the sample.

----------------------------------------------

Notes:  In the sample, You can change the VO config to let screen light.
        In the sample, you maybe need to change the vcap config to adapt the sensor.



