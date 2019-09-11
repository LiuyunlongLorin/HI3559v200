Usage:

1. Run script of loadko first, in mpp/out/linux/xxx/ko/.
2. Run make in directory datafifo/sample

3. At single serial, Input command 'sample_writer'. 
   It will give the physic adress of datafifo:
       PhyAddr: 9326a000
       datafifo_init finish
       press any key to start.
4. At big-little serial, Input command './sample_reader 9326a000'.
   It will output:
        u64PhyAddr==9326a000
        datafifo_init finish
        press any key to start.
   Then press Enter, start to read data.
5. At single serial, press Enter to write data.
6. Press 'q' to exit data transfer.
7. Then press Enter exit sample.