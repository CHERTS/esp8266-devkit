from distutils.core import setup
import py2exe
     
setup(
    name='esptool',
    description='A utility to communicate with the ROM bootloader in Espressif ESP8266.',
    version='0.1.0',
    license='GPLv2+',
    author='Fredrik Ahlberg',
    author_email='fredrik@z80.se',
    url='https://github.com/themadinventor/esptool',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Embedded Systems',
        'Environment :: Console',
        'License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)',
        'Programming Language :: Python :: 2.7',
    ],
    console=[{'script': 'esptool.py'}],
    options={'py2exe': {'includes':['binascii','re','string','sys','serial','argparse'],'optimize':2}}
)
