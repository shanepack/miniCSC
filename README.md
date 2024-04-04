# miniCSC - ROOT, Scripts & Help
Cataloged work on the miniCSC project by Shane Pack & Dylan Parks. 

## Getting ROOT Installed Locally:

**You can choose to either use ROOT with C++ or Python. _We used C++!_**

* **Windows 10/11**:
  
    You can [install ROOT as an executable on Windows](https://root.cern/install/all_releases/), but it is recommended to use WSL.

    - [YouTube Tutorial on ROOT with WSL Setup by 'Physics Matters'](https://youtu.be/pmfM4Zq6OQU?si=UIo0tsXVN-lnjHhA)

* **Linux & MacOS**

    [Official ROOT Install Guide](https://root.cern/install/)

## Learning ROOT Resources:

There are a LOT of possibilities (and libraries) associated with ROOT, these are the resources we found most helpful:

  - [Official ROOT Tutorials](https://root.cern/doc/master/group__Tutorials.html)
  - [Offcial ROOT Manual](https://root.cern/manual/functional_parts/)
  - [Offcial ROOT Reference Documentation](https://root.cern/doc/master/index.html)


  > [!TIP]
  >  _If you need help with C++, try using [w3schools](https://www.w3schools.com/cpp/default.asp) or [ChatGPT Pro (Paid)](https://chat.openai.com/) (This can also help you with ROOT!)_

## LXPLUS Access & Data Aquisition:

  Accessing LXPLUS and getting setup to properly perform DAQ and retrieve data from tests run on the miniCSCs can be a bit tricky, 
  so we highly recommend following [our step-by-step setup guide available on the 'CSCProto@904' TWiki page.](https://twiki.cern.ch/twiki/bin/view/CMS/DAQminiCSC)

  > [!TIP]
  >  _If you're unfamiliar with Linux terminal commands, it's recommended to refer to this [cheat sheet]([https://www.guru99.com/linux-commands-cheat-sheet.html](https://github.com/santosh373/Linux-Basics/blob/master/linux%20cheat%20sheet.pdf))!_

## Data Logging:

  Data from every test we performed on the miniCSC's are available in [this spreadsheet](https://docs.google.com/spreadsheets/d/1zSMEwGt_I1K-cLdwoXqq-PGu8aNxlSe0PZry_535MEk/edit?usp=sharing).

  **If you're interested in adding test data to/creating your own spreadsheet, use our [TMB Dump Python Script](https://github.com/shanepack/miniCSC/blob/main/tmb_dump_script.py) which is capable of generating:**
  
  - Plain text for analysis _([Example](https://twiki.cern.ch/twiki/pub/CMS/MiniCSCLogScript/elog_out.txt))_ 
  - HTML formatted text for eLog posts _([Example](https://twiki.cern.ch/twiki/pub/CMS/MiniCSCLogScript/HTML_EXAMPLE_elog_out.txt))_ 
  - .csv formatted text for pasting into spreadsheets _([Example](https://twiki.cern.ch/twiki/pub/CMS/MiniCSCLogScript/csv_out.txt))_ 

  > [!IMPORTANT]
  > **_Our TWiki post on how to use the script can be found [here.](https://twiki.cern.ch/twiki/bin/view/CMS/MiniCSCLogScript)_** 
  
