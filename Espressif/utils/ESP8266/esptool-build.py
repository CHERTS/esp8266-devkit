from distutils.core import setup
import py2exe
     
setup(
    name='esptool',
    description='A utility to communicate with the ROM bootloader in Espressif ESP8266.',
    version='0.1.0',
    license='GPLv2+',
    author='Fredrik Ahlberg (themadinventor) & Angus Gratton (projectgus)',
    author_email='gus@projectgus.com',
    url='https://github.com/themadinventor/esptool',
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'Operating System :: POSIX',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: MacOS :: MacOS X',
        'Topic :: Software Development :: Embedded Systems',
        'Environment :: Console',
        'License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)',
        'Programming Language :: Python :: 2.7',
    ],
    console=[{'script': 'esptool.py'}],
    options={'py2exe': {'includes':['binascii','re','string','sys','serial','argparse','json'],'optimize':2}}
)
