# trace_sip
---

### An opensips module used to trace sip information from opensips

trace_sip module aiming to trace sip information from opensips and 
store them to file or redis channel.
contact address: zjbhuxian@gmail.com.

### Usage

	...
	route{
		trace_sip();
	}
	...


### Features

- Log to file
- Log to redis channel

### Installation

	1) download source to opensips-2.4.6/module/trace_sip/
	2) make
	3) cp trace_sip.so /opensips_install_path/lib64/opensips/modules/
	4) cp trace_sip.cfg /opensips_install_path/etc/opensips/
	5) modify trace_sip.cfg

### Notes

Based opensips-2.4.6
