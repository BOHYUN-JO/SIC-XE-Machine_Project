# SIC-XE-Machine_Project 소개

이 프로젝트는 다음과 같은 SIC/XE 머신의 기능(어셈블러, 컴파일러, 링커, 로더)을 구현한다. 
## 1. 간단한 Shell 명령, opcode table 출력
```
- h[elp] : Shell에서 실행 가능한 모든 명령어들의 리스트를 화면에 출력
- d[ir] : 현재 directory에 있는 파일들을 출력
- q[uit] : Shell을 종료
- hi[story] : 현재까지 사용한 명령어들으 순서대로 번호와 함께 출력
- du[mp] [start, end] : 할당되어 있는 메모리의 내용으 출력(메모리 주소, 메모리내용(16진수); 메모리 내용(ASCII, 20~7E))
- e[dit] address, value : 메모리의 address번지 값을 value에 지정된 값으로 변경
- f[ill] start, end, value : 메모리의 start번지부 end번지 까지의 값을 value에 지정된 값으로 변경
- reset : 메모리 전체를 전부 0을 변경
- opcode mnemonic: 명령어에 해당하는 opcode를 출력
- opcodelist: opcode hash table의 내용을 출력
```

## 2. Assemble 기능
```
- type filename: filename에 해당하는 파일을 현재 디렉토리에서 읽어서 화면에 출력
- assemble filename: filename에 햐당하는 소스파일을 읽어서 object파일과 리스팅 파일을 생성
- symbol: 가자 최근의 assemble 과정에서 생성된 symbol table을 화면에 출력
```

## 3. Linking, Loading 기능
```
- progaddr [address] : loader 또는 run 명령어르 수행할 때 시작하는 주소를 지정
- loader [object filename][object filename2] [...] : filename1, 2, ...에 해당하는 파일으 읽어서 linking 작업을 수해 후 가사 메모리에 그 결과를 기록
- bp [address]: breakpoint르 지정
- bp clear: 존재하는 breakpoint를 전부 삭제
- bp: 존재하는 breakpoint르 전부 화면에 출력
- run: 메모리에 load된 프로그램의 breakpoint까지 실행, breakpoint가 없으면 끝까지 실행
```

# 실행환경
Ubuntu 16.04.2 LTS (GNU/Linux 4.4.0-193-generic x86_64)

# 컴파일 및 실행 방법
```
> make
> ./20160641.out
```

# 실행예시
## 1. 간단한 Shell 명령, opcode table 출력
![image](https://user-images.githubusercontent.com/70252973/135466792-b89603bb-2ba5-4912-8660-9dcc24cc7fe2.png)
![image](https://user-images.githubusercontent.com/70252973/135467076-540e1b09-95bd-4857-94b1-8fa5f6d77371.png)
## 2. Assemble 기능
![image](https://user-images.githubusercontent.com/70252973/135467130-e2959c8a-c814-4229-b748-dbbdaebacfcd.png)
![image](https://user-images.githubusercontent.com/70252973/135467221-ad4a47ba-b9d0-4960-b62b-585279891738.png)
![image](https://user-images.githubusercontent.com/70252973/135467262-80885dc3-0790-4d5c-abcf-8d3faf3fdd8f.png)
## 3. Linking, Loading 기능
![image](https://user-images.githubusercontent.com/70252973/135467333-a9977d7c-4afd-48e3-b19e-e86dbf1cd2fb.png)
![image](https://user-images.githubusercontent.com/70252973/135467363-713d8d55-dd23-4cd7-94c1-8f31dd953aa3.png)
