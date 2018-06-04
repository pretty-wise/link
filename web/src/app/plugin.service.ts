import { Injectable } from '@angular/core';
import { Plugin } from './shared/plugin';
import { Observable, of, forkJoin } from 'rxjs';
import { map } from "rxjs/operators";
import { HttpClient } from '@angular/common/http';


export interface CommandListResponse {
  plugin: Plugin;
  commands: string[];
}

export interface PluginListResponse {
  master: boolean;
  plugins: Plugin[];
}

@Injectable({
  providedIn: 'root'
})
export class PluginService {

  constructor(
    private http: HttpClient) { }

  COMMAND_LIST_URI: string = '/command-list';

  plugins: Plugin[] = [
    { name: 'test', version: '0.1', hostname: '1.1.1.1', port: 0, pid: 0 },
    { name: 'test2', version: '0.2', hostname: '1.1.1.1', port: 0, pid: 0 },
    { name: 'test3', version: '0.2', hostname: '1.1.1.1', port: 0, pid: 0 }
  ];

  getPluginList(commands: string[]) : Observable<Plugin[]> {
    console.log('cmds: ' + commands);
    var httpRequests = commands.map(cmd => this.http.get<PluginListResponse>(cmd));
    console.log(httpRequests.length);
    const allGetRequests = forkJoin(httpRequests);
    return allGetRequests.pipe(map((responses: PluginListResponse[]) => {
      console.log(responses);
      var masterList = responses.find(function(element: PluginListResponse) { return element.master == true; });

      if(masterList === undefined) {
        return []; // no master list found.
      }
      return masterList.plugins;
    }));
  }

  getCommandList(hostname: string) : Observable<string[]> {
    return this.http.get<CommandListResponse>(hostname + this.COMMAND_LIST_URI).pipe(map((response: CommandListResponse)  => {
      return response.commands;
    }));
  }
}
