import time

# 400 Bad Request를 Bad Request라고 적어서 응답했더니 브라우저에서는 200 처리했었음
STATUS_CODE = {
    200: '200 OK',
    201: '201 Created',
    202: '202 Accepted',
    400: '400 Bad Request',
    404: '404 Not Found',
    405: '405 Method Not Allowed',
    409: '409 Conflict',
    422: '422 Unprocessable Entity',
}

VERSION = 'HTTP/1.1'


def create_header(code, option):
    header = list()

    header.append(f'{VERSION} {STATUS_CODE[code]}')
    header.append(f'Server: Network Project')
    # header.append(f'Connection: close')
    header.append(f'Access-Control-Allow-Origin: *')
    header.append(f'Date: {time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime())}')

    for (k, v) in option.items():
        header.append(f'{k}: {v}')

    return '\r\n'.join(header)


def create_response(code, content=None, is_preflight=False, preflight_option=None):
    option = dict()
    if content is not None:
        option['Content-Type'] = 'application/json'
        option['Content-Length'] = len(content)
    else:
        content = ''

    if is_preflight:
        option['Access-Control-Allow-Methods'] = ', '.join(preflight_option['method'])
        if (allow_headers := preflight_option.get('header')) is not None:
            option['Access-Control-Allow-Headers'] = ', '.join(allow_headers)
        option['Access-Control-Max-Age'] = 86400

    header = create_header(code, option)
    packet = '\r\n\r\n'.join([header, content])

    return packet.encode()
