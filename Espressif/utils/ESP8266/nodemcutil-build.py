from distutils.core import setup
import py2exe
     
setup(
    name='nodemcutil',
    description='File manipulation utility for NodeMCU firmware for ESP8266 WiFi modules.',
    version='0.1.0',
    license='GPLv3+',
    author='Tamas Szabo',
    author_email='sza2trash@gmail.com',
    url='https://github.com/sza2/nodemcu_file_util',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Embedded Systems',
        'Environment :: Console',
        'License :: OSI Approved :: GNU General Public License v2 or later (GPLv2+)',
        'Programming Language :: Python :: 2.7',
    ],
    console=[{'script': 'nodemcutil.py'}],
    options={'py2exe': {'includes':['binascii','re','os','re','string','sys','serial','argparse','json','hashlib','inspect','base64','zlib','shlex','time','struct'],'optimize':2}}
)
