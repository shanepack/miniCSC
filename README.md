# miniCSC - ROOT, Scripts & Help
Cataloged work for use by future researchers on the CSC Prototype (miniCSC) project at CERN by Shane Pack & Dylan Parks.

## Introduction:

To familiarize yourself with Cathode Strip Chambers (CSCs) in the CMS, and the motivation for the CSC Prototypes (aptly named 'miniCSCs'), we recommend reading the following material:
  - [Longevity studies of CSC prototypes operating with Ar+CO 2 gas mixture and different fractions of CF 4](https://link.springer.com/content/pdf/10.1140/epjp/s13360-023-04679-7.pdf) 
  - [Searches for CF4 replacement for the CSC gas mixture](https://indico.cern.ch/event/1022051/contributions/4319536/attachments/2231279/3780771/CSCsearchNewGas_210422_MiniWorkshop.pdf) (This details the primary goal of the research!)

Don't worry if the contents of the paper and slides are difficult to follow, you'll gain an understanding of the unfamiliar terms and concepts as you are introduced to the project!


  - The following sections are dedicated to personal system set-up for **future miniCSC researchers**, focusing primarily on setting up ROOT. 

***Note:** Access to the data acquisition guides on tWiki requires a **'CERN Single Sign-On'** login.*

## Getting ROOT (C++ version) Installed Locally:

**You can choose to either use ROOT with C++ or Python. _We used C++!_**

* **Windows 10/11**:
  
    You can [install ROOT as an executable on Windows](https://root.cern/install/all_releases/), but it is recommended to use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install).

    - [YouTube Tutorial on ROOT with WSL Setup by 'Physics Matters'](https://youtu.be/pmfM4Zq6OQU?si=UIo0tsXVN-lnjHhA)

* **Linux & MacOS**

    [Official ROOT Install Guide](https://root.cern/install/)

***Note:** You can use ROOT with Python with PyROOT, however, **the content provided in this repository was completed using ROOT for C++.***

## Learning ROOT Resources:

There are a lot of possibilities (and libraries) associated with ROOT, these are the resources we found most helpful:

  - [Official ROOT Tutorials](https://root.cern/doc/master/group__Tutorials.html)
  - [Offcial ROOT Manual](https://root.cern/manual/functional_parts/)
  - [Offcial ROOT Reference Documentation](https://root.cern/doc/master/index.html)


  > [!TIP]
  >  _If you need help with C++, try using [w3schools](https://www.w3schools.com/cpp/default.asp) or [ChatGPT Pro (Paid)](https://chat.openai.com/) (This can also help you with ROOT!)_

## LXPLUS Access & Data Aquisition:

  Accessing LXPLUS and getting set up to properly perform DAQ and retrieve data from tests run on the miniCSCs can be a bit tricky, 
  so we highly recommend following [our step-by-step setup guide available on the 'CSCProto@904' TWiki page.](https://twiki.cern.ch/twiki/bin/view/CMS/DAQminiCSC)

  > [!TIP]
  >  _If you're unfamiliar with Linux terminal commands, it's recommended to refer to this [cheat sheet](https://www.geeksforgeeks.org/linux-commands-cheat-sheet/)!_

## Data Logging:

  Data from every test we performed on the miniCSCs are available in [this spreadsheet](https://docs.google.com/spreadsheets/d/1zSMEwGt_I1K-cLdwoXqq-PGu8aNxlSe0PZry_535MEk/edit?usp=sharing).

  **If you're interested in adding test data to/creating your own spreadsheet, use our [TMB Dump Python Script](https://github.com/shanepack/miniCSC/blob/main/tmb_dump_script.py) which is capable of generating:**
  
  - Plain text for analysis _([Example](https://twiki.cern.ch/twiki/pub/CMS/MiniCSCLogScript/elog_out.txt))_ 
  - HTML formatted text for eLog posts _([Example](https://twiki.cern.ch/twiki/pub/CMS/MiniCSCLogScript/HTML_EXAMPLE_elog_out.txt))_ 
  - .csv formatted text for pasting into spreadsheets _([Example](https://twiki.cern.ch/twiki/pub/CMS/MiniCSCLogScript/csv_out.txt))_ 

  > [!IMPORTANT]
  > **_Our TWiki post on how to use the script can be found [here.](https://twiki.cern.ch/twiki/bin/view/CMS/MiniCSCLogScript)_**
  
